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


namespace portfolio {

    class PortfolioRiskManager {
        
        private:
        unordered_map<uint64_t, std::unique_ptr<orders::Order>> orders;
        unordered_map<uint16_t, vector<uint64_t>> userOrders;
        unordered_map<uint16_t, std::unique_ptr<instrument::FinancialInstrument>> instruments;
        int BUY_THRESHOLD;
        int SELL_THRESHOLD;

        void createResponse(messageSpecs::OrderResponse &orderResponse, const bool &acceptanceStatus, const uint64_t &orderId) {
            orderResponse.messageType = messageSpecs::OrderResponse::MESSAGE_TYPE;
            orderResponse.orderId = orderId;
            orderResponse.status = acceptanceStatus? messageSpecs::OrderResponse::Status::ACCEPTED : messageSpecs::OrderResponse::Status::REJECTED;
        }

        public:
        PortfolioRiskManager():orders({}), userOrders({}), instruments({}),BUY_THRESHOLD(20),SELL_THRESHOLD(15){

        }

        void newOrder(const int &sd, const messageSpecs::NewOrder &newOrder, messageSpecs::OrderResponse &orderResponse) {

            unique_ptr<orders::Order> order = std::make_unique<orders::Order>(newOrder.orderId, newOrder.listingId, newOrder.orderQuantity, newOrder.orderPrice, newOrder.side);
            if(instruments.find(newOrder.listingId) == instruments.end()) {
                cout<<"Adding new instrument with ID: "<<newOrder.listingId<<"\n";
                instruments[newOrder.listingId] = std::make_unique<instrument::FinancialInstrument>();
            }

            unique_ptr<instrument::FinancialInstrument> &instrument = instruments[newOrder.listingId];

            bool acceptanceStatus = instrument->assessAddOrder(order);
    
            createResponse(orderResponse, acceptanceStatus, newOrder.orderId);

            if(acceptanceStatus) {
                orders[order->orderId] = std::move(order);
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
        }

        void deleteOrder(const messageSpecs::DeleteOrder &deleteOrder) {
            
            if(orders.find(deleteOrder.orderId) == orders.end()) {
                std::cerr<<"Order with order ID: "<<deleteOrder.orderId<<" already deleted or no such order exists, can't delete. \n";
                return;
            }

            unique_ptr<orders::Order> &order = orders[deleteOrder.orderId];
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

        void modifyOrder(const messageSpecs::ModifyOrderQuantity &modifyOrderQuantity, messageSpecs::OrderResponse &orderResponse) {
            
            if(orders.find(modifyOrderQuantity.orderId) == orders.end()) {
                std::cerr<<"Modify instruction for order with order ID: "<<modifyOrderQuantity.orderId<<" doesn't exist. \n";
                return;                
            }

            unique_ptr<orders::Order> &order = orders[modifyOrderQuantity.orderId];

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

            return;

        }

        void trade(const messageSpecs::Trade &trade) {

            if(orders.find(trade.tradeId) == orders.end()) {
                std::cerr<<"Trade instruction for order with order ID: "<<trade.tradeId<<" doesn't exist. \n";
                return;     
            }

            unique_ptr<orders::Order> &order = orders[trade.tradeId];
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

        void deleteUser(const uint64_t &userId) {
            cout<<"------------------------------\n";
            cout<<"Discarding orders for user ID: "<<userId<<"\n";           
            for(uint64_t &orderId: userOrders[userId]) {
                if(orders.find(orderId)!= orders.end()) {
                    unique_ptr<orders::Order> &order = orders[orderId];
                    unique_ptr<instrument::FinancialInstrument> &instrument = instruments[order->instrumentId];
                    instrument->deleteOrder(order);
                    orders.erase(orderId);                    
                }
            }

            userOrders.erase(userId);
            cout<<"User deleted \n";
            cout<<"------------------------------\n"; 

            return;
        }


    };

}