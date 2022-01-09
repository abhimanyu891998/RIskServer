//Socket code has been taken from geeksforgeeks https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/?ref=lbp
#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include<iostream>
#include<memory>
#include<chrono>
#include "../messageStructures.hpp"
#include "../PortfolioRiskManager/PortfolioRiskManager.hpp"
#include "../Config/config.hpp"

using std::cout;
using std::cin;
using std::unique_ptr;
     
#define TRUE   1 
#define FALSE  0 
#define PORT 51717 


/**
 * Creates an orderResponse message to be sent to the client.
**/
char* createMessage(const messageSpecs::OrderResponse &response, const uint32_t &currentSequenceNumber) {
    char *message = new char[28];
    messageSpecs::Header header;
    header.payloadSize = 12;
    header.sequenceNumber = currentSequenceNumber+1;
    header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    std::memcpy(message, &header, 16);
    std::memcpy(message+16, &response, 12);

    return message;

}

/**
 * Parses the message from client and does the processing accordingly.
**/
void parseMessage(const int &sd, int &valread, char* buffer, unique_ptr<portfolio::PortfolioRiskManager> &riskManager) {

    messageSpecs::Header header;
    std::memcpy(&header, buffer, 16);
    valread = read(sd, buffer, header.payloadSize);

    uint16_t *messageType = (uint16_t*) buffer;
    cout<<"Message type is: "<<*messageType<<"\n";
    char *message;
    switch(*messageType) {
        case messageSpecs::NewOrder::MESSAGE_TYPE: { messageSpecs::NewOrder newOrder;
                                                   std::memcpy(&newOrder, buffer, header.payloadSize);
                                                   messageSpecs::OrderResponse response = riskManager->newOrder(sd, newOrder);
                                                   message = createMessage(response, header.sequenceNumber);
                                                   cout<<"Sending message to the client \n";
                                                   send(sd,message,28,0);
                                                   break;
                                                   }

        case messageSpecs::DeleteOrder::MESSAGE_TYPE: { messageSpecs::DeleteOrder orderToBeDeleted;
                                                      std::memcpy(&orderToBeDeleted, buffer, header.payloadSize);
                                                      riskManager->deleteOrder(orderToBeDeleted);
                                                      break;
                                                      }

        case messageSpecs::ModifyOrderQuantity::MESSAGE_TYPE: { messageSpecs::ModifyOrderQuantity modifyOrderQuantity;
                                                              std::memcpy(&modifyOrderQuantity, buffer, header.payloadSize);
                                                              messageSpecs::OrderResponse response = riskManager->modifyOrder(modifyOrderQuantity);
                                                              message = createMessage(response, header.sequenceNumber);
                                                              cout<<"Sending message to the client \n";
                                                              send(sd,message,28,0);
                                                              break;
                                                              }

        case messageSpecs::Trade::MESSAGE_TYPE: { messageSpecs::Trade trade;
                                                std::memcpy(&trade, buffer, header.payloadSize);
                                                cout<<"hhhh\n";
                                                cout<<trade.listingId<<std::endl;
                                                cout<<trade.tradePrice<<std::endl;
                                                riskManager->trade(trade);
                                                break;
                                                }


        
    }

    return;

}




int main(int argc , char *argv[])  
{  
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;  
    int max_sd;  
    struct sockaddr_in address;  
         
    char buffer[1025];  //data buffer of 1K
    configuration::Config config;

    //Setting default values
    config.BUY_THRESHOLD = 20; 
    config.SELL_THRESHOLD = 15;

    if(argc>=2)
        cout<<"Setting new buy threshold "<<std::atoi(argv[1])<<std::endl;
        config.BUY_THRESHOLD = std::atoi(argv[1]);

    if(argc>=3)
        cout<<"Setting new sell threshold "<<std::atoi(argv[2])<<std::endl;
        config.SELL_THRESHOLD = std::atoi(argv[2]);
        
    
    unique_ptr<portfolio::PortfolioRiskManager> riskManager = std::make_unique<portfolio::PortfolioRiskManager>(config); 
         
    //set of socket descriptors 
    fd_set readfds;   
         
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  
         
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
     
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
     
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
         
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
         
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
         
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
         
    while(TRUE)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
     
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        } 

             
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
                
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                         
                    break;  
                }  
            }


        }  
             
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = client_socket[i];  
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 16)) <= 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Handle disconnected user
                    riskManager->deleteUser(sd);

                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                     
                else 
                {  
                    cout<<valread<<" bytes read \n";
                    parseMessage(sd,valread,buffer,riskManager); //Handle the incoming messages from clients
                    cout<<"\n";
                }

            }  
        }  
    }  
         
    return 0;  
} 