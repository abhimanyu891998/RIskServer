// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include<iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include<chrono>
#include "../messageStructures.hpp"

#define PORT 51717

using std::cout;
using std::cin;
using std::memcpy;

uint32_t lastSequenceNumber = 0;

/**
 * Creates header for the message to be sent to the server based on the messageType
**/
void populateHeader(messageSpecs::Header &header, const int &messageType) {

    switch(messageType) {
        case 1: {
            header.version = 0;
            header.payloadSize = 35;
            lastSequenceNumber+=1;
            header.sequenceNumber = lastSequenceNumber;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
            break;           
        }

        case 2: {
            header.version = 0;
            header.payloadSize = 10;
            lastSequenceNumber+=1;
            header.sequenceNumber = lastSequenceNumber;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
            break;            
        }

        case 3: {
            header.version = 0;
            header.payloadSize = 18;
            lastSequenceNumber+=1;
            header.sequenceNumber = lastSequenceNumber;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
            break;
        }

        case 4: {
            header.version = 0;
            header.payloadSize = 34;
            lastSequenceNumber+=1;
            header.sequenceNumber = lastSequenceNumber;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
            break;            
        }
    }

    return;
}

/**
 * Creates message to be sent to the server based on the messageType and returns a pair of message and message payload size.
**/
std::pair<char*,int> createMessage(const int &messageType) {
    char* message;
    messageSpecs::Header header;
    switch(messageType) {
        case 1: {
            messageSpecs::NewOrder newOrder;
            cout<<"Listing ID:\n";
            cin>>newOrder.listingId;
            cout<<"Order ID:\n";
            cin>>newOrder.orderId;
            cout<<"Order Price:\n";
            cin>>newOrder.orderPrice;
            cout<<"Side (B/S): \n";
            cin>>newOrder.side;
            cout<<"Order Quantity: \n";
            cin>>newOrder.orderQuantity;
            newOrder.messageType = 1;
            //Header
            populateHeader(header, newOrder.messageType);
            //Message creation
            message = new char[51];
            memcpy(message, &header, 16);
            memcpy(message+16, &newOrder, 35);
            break;
        }

        case 2: {
            messageSpecs::DeleteOrder deleteOrder;
            cout<<"Order ID: \n";
            cin>>deleteOrder.orderId;
            deleteOrder.messageType = 2;
            //Header
            populateHeader(header, deleteOrder.messageType);
            //Messsage Creation
            message = new char[26];
            memcpy(message, &header, 16);
            memcpy(message+16, &deleteOrder, 10);
            break;
        }

        case 3: {
            messageSpecs::ModifyOrderQuantity modifyOrderQuantity;
            cout<<"Order ID: \n";
            cin>>modifyOrderQuantity.orderId;
            cout<<"New quantity: \n";
            cin>>modifyOrderQuantity.newQuantity;
            modifyOrderQuantity.messageType = 3;
            //Header
            populateHeader(header, 3);
            message = new char[34];
            memcpy(message, &header, 16);
            memcpy(message+16, &modifyOrderQuantity, 18);
            break;
        }

        case 4: {
            messageSpecs::Trade trade;
            cout<<"Listing ID: \n";
            cin>>trade.listingId;
            cout<<"Trade Quantity: \n";
            cin>>trade.tradeQuantity;
            cout<<"Trade Price: \n";
            cin>>trade.tradePrice;
            cout<<"Trade ID: \n";
            cin>>trade.tradeId;
            trade.messageType = 4;
            //Header
            populateHeader(header, 4);
            message = new char[50];
            memcpy(message, &header, 16);
            memcpy(message+16, &trade, 34);
            break;
        }

        default: {
            std::cerr<<"Invalid message type passed to create message";
            message = new char[0];
        }

    }

    return {message, header.payloadSize};
}

/**
 * Client side of the program, it runs in an infinite loop and keeps accepting the instructions till the user disconnects.
**/
int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1025];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    bool keepUp = true;
    int messageType;

    //Runs till disconnect is issued.
    while(keepUp) {
        cout<<"Choose from the following message types and press the number corresponding to the type: \n";
        cout<<"1 - New Order \n 2 - Delete Order \n 3 - Modify Order \n 4 - Trade\n 6 - Disconnect\n";
        cin>>messageType;
        if(messageType == 6) {
            keepUp = false; //disconnect
        }
        else {
        std::pair<char*, int> messageObject = createMessage(messageType);
        char *message = messageObject.first;
        int payloadSize = messageObject.second;
        //Sending message to server
        send(sock, message, 16+payloadSize, 0); //Sends the message to server

        // If the type is add new order or modify and order, we expect a response from the server
        if(messageType == 1 || messageType == 3) {
            //No response received from server
            if ((valread = read( sock , buffer, 16)) <= 0) {
                std::cerr<<"Server disconnected";
                break;
            }

            messageSpecs::Header header;
            memcpy(&header, buffer, 16); //Reading header of the response

            lastSequenceNumber = header.sequenceNumber;
            valread = read(sock, buffer, header.payloadSize);
            messageSpecs::OrderResponse response;
            memcpy(&response, buffer, header.payloadSize);

            cout<<"Response from server for the previous request with order ID: "<<response.orderId<<" is: ";
            if(response.status == messageSpecs::OrderResponse::Status::ACCEPTED)
                cout<<"Accepted\n";
            else
                cout<<"Rejected \n";

            cout<<std::endl;
        } 

        }

    }

    return 0;
}