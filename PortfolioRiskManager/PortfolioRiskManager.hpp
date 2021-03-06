#pragma once

#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<unordered_map>
#include "../messageStructures.hpp"
#include "../Order/Order.hpp"
#include "../FinancialInstrument/FinancialInstrument.hpp"

using std::cout;
using std::cin;
using std::vector;
using std::unordered_map;
using std::unique_ptr;

/**
 * Risk manager for the overall portfolio, it stores the hash map of orders, maps users to their orders, and stores information about instruments.
 **/
namespace portfolio {

    class PortfolioRiskManager {
        
        private:
        unordered_map<uint64_t, std::shared_ptr<orders::Order>> orders; //Order ID to order map
        unordered_map<uint16_t, vector<uint64_t>> userOrders; //User id (socket descriptor) to order ID map
        unordered_map<uint16_t, std::unique_ptr<instrument::FinancialInstrument>> instruments; //Instrument ID mapped to the instruments

        //For creating the order response from server to client.
        void createResponse(messageSpecs::OrderResponse &orderResponse, const bool &acceptanceStatus, const uint64_t &orderId) {
            orderResponse.messageType = messageSpecs::OrderResponse::MESSAGE_TYPE;
            orderResponse.orderId = orderId;
            orderResponse.status = acceptanceStatus? messageSpecs::OrderResponse::Status::ACCEPTED : messageSpecs::OrderResponse::Status::REJECTED;
            return;
        }

        public:
        //Contains details of thresholds and other configuration parameters can be added.
        configuration::Config config;

        PortfolioRiskManager(const configuration::Config &config):orders({}), userOrders({}), config(config){

        }

        // Handles a new order, checks if the instrument is new, adds it to the list as well. Changes the state of system based on the acceptanceStatus.
        messageSpecs::OrderResponse newOrder(const int &sd, const messageSpecs::NewOrder &newOrder) {

            std::shared_ptr<orders::Order> order = std::make_shared<orders::Order>(newOrder.orderId, newOrder.listingId, newOrder.orderQuantity, newOrder.orderPrice, newOrder.side);
            if(instruments.find(newOrder.listingId) == instruments.end()) {
                cout<<"Adding new instrument with ID: "<<newOrder.listingId<<"\n";
                instruments[newOrder.listingId] = std::make_unique<instrument::FinancialInstrument>(config);
            }

            unique_ptr<instrument::FinancialInstrument> &instrument = instruments[newOrder.listingId];

            bool acceptanceStatus = instrument->assessAddOrder(order);
            
            messageSpecs::OrderResponse orderResponse;
            createResponse(orderResponse, acceptanceStatus, newOrder.orderId);

            if(acceptanceStatus) {
                orders[order->orderId] = order;
                userOrders[sd].push_back(order->orderId);
            }

            cout<<"------------------------------\n";
            cout<<"Details of new order: \n";
            cout<<"ID: "<<newOrder.orderId<<"\n";
            cout<<"Instrument ID: "<<newOrder.listingId<<"\n";
            cout<<"Quantity: "<<newOrder.orderQuantity<<"\n";
            cout<<"Price: "<<newOrder.orderPrice<<"\n";
            cout<<"Side: "<<newOrder.side<<"\n";
            cout<<"Status: "<<(acceptanceStatus?"Accepted" : "Rejected")<<"\n";
            cout<<"------------------------------\n";

            return orderResponse;
        }

        //Handles order deletion.
        void deleteOrder(const messageSpecs::DeleteOrder &deleteOrder) {
            
            if(orders.find(deleteOrder.orderId) == orders.end()) {
                std::cerr<<"Order with order ID: "<<deleteOrder.orderId<<" already deleted or no such order exists, can't delete. \n";
                return;
            }

            std::shared_ptr<orders::Order> order = orders[deleteOrder.orderId];
            unique_ptr<instrument::FinancialInstrument> &instrument = instruments[order->instrumentId];

            instrument->deleteOrder(order);

            cout<<"------------------------------\n";
            cout<<"Deleting order: \n";
            cout<<"ID: "<<order->orderId<<"\n";
            cout<<"Instrument ID: "<<order->instrumentId<<"\n";
            cout<<"Quantity: "<<order->quantity<<"\n";
            cout<<"Price: "<<order->price<<"\n";
            cout<<"Side: "<<order->side<<"\n";
            cout<<"\n";
            cout<<"------------------------------\n";
            
            orders.erase(order->orderId);

            return;
        }

        //Modifies order if it is acceptable to do so, sends a response to the client if the modification request was accepted or not.
        messageSpecs::OrderResponse modifyOrder(const messageSpecs::ModifyOrderQuantity &modifyOrderQuantity) {
            messageSpecs::OrderResponse orderResponse;
            if(orders.find(modifyOrderQuantity.orderId) == orders.end()) {
                std::cerr<<"Modify instruction for order with order ID: "<<modifyOrderQuantity.orderId<<" doesn't exist. \n";
                createResponse(orderResponse, false, modifyOrderQuantity.orderId);
                return orderResponse;
            }

            std::shared_ptr<orders::Order> order = orders[modifyOrderQuantity.orderId];

            cout<<"------------------------------\n";
            cout<<"Attempting to modify order: \n";
            cout<<"Order ID: "<<order->orderId<<"\n";
            cout<<"Old Quantity: "<<order->quantity<<"\n";
            cout<<"New Quantity: "<<modifyOrderQuantity.newQuantity<<"\n";


            unique_ptr<instrument::FinancialInstrument> &instrument = instruments[order->instrumentId];

            bool acceptanceStatus = instrument->assessModifyOrder(order, modifyOrderQuantity.newQuantity);

            if(acceptanceStatus) {
                order->quantity = modifyOrderQuantity.newQuantity;
            }

            
            createResponse(orderResponse, acceptanceStatus, modifyOrderQuantity.orderId);

            cout<<"Modification status: "<<(acceptanceStatus?"Accepted" : "Rejected")<<"\n";
            cout<<"------------------------------\n";

            return orderResponse;

        }

        //Updates the net position based on trade information for the instrument and the order.
        void trade(const messageSpecs::Trade &trade) {

            if(orders.find(trade.tradeId) == orders.end()) {
                std::cerr<<"Trade instruction for order with order ID: "<<trade.tradeId<<" doesn't exist. \n";
                return;     
            }

            std::shared_ptr<orders::Order> order = orders[trade.tradeId];
            unique_ptr<instrument::FinancialInstrument> &instrument = instruments[trade.listingId];

            instrument->handleTrade(trade.tradeQuantity, order->side);


            cout<<"------------------------------\n";
            cout<<"Trade handled: \n";
            cout<<"Order ID: "<<trade.tradeId<<"\n";
            cout<<"Instrument ID: "<<trade.listingId<<"\n";
            cout<<"Side: "<<order->side<<"\n";
            cout<<"Quantity: "<<trade.tradeQuantity<<"\n";
            cout<<"Trade Price: "<<trade.tradePrice<<"\n";
            cout<<"------------------------------\n";
            
            return;
        }

        //Deletes the user and their associated orders.
        void deleteUser(const uint64_t &userId) {

            cout<<"------------------------------\n";
            cout<<"Discarding orders for user ID: "<<userId<<"\n";   

            if(userOrders.find(userId) == userOrders.end()) {
                cout<<"User has no orders, nothing to delete.\n";
            }
            else {
                for(uint64_t &orderId: userOrders[userId]) {
                    if(orders.find(orderId)!= orders.end()) {
                        std::shared_ptr<orders::Order> order = orders[orderId];
                        unique_ptr<instrument::FinancialInstrument> &instrument = instruments[order->instrumentId];
                        instrument->deleteOrder(order);
                        orders.erase(orderId);                    
                    }
                }
                userOrders.erase(userId);
                cout<<"User deleted \n";
            }
            cout<<"------------------------------\n"; 

            return;
        }


    };

}