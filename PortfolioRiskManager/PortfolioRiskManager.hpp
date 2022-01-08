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
        unorderd_map<uint16_t, vector<uint64_t>> userOrders;
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
            cout<<"Status: "<<(res?"Accepted" : "Rejected")<<"\n";
            cout<<"------------------------------\n";
        }

        void deleteOrder(const messageSpecs::DeleteOrder &deleteOrder) {

        }

        void modifyOrder(const messageSpecs::ModifyOrderQuantity &modifyOrderQuantity) {

        }

        void trade(const messageSpecs::Trade &trade) {

        }

        void deleteUser(const uint64_t &userId) {

        }


    };

}