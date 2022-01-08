#pragma once

#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<unordered_map>
#include "../messageStructures.hpp"
#include "../Order/Order.hpp"

using std::cout;
using std::cin;
using std::vector;
using std::unordered_map;

namespace instrument {

    class FinancialInstrument {

        private:
        uint64_t netPos;
        uint64_t buyQty;
        uint64_t sellQty;
        uint64_t buyHypotheticalWorst;
        uint64_t sellHypotheticalWorst;


        uint64_t calculateBuyHypotheticalWorst() {
            return std::max(buyQty, netPos+buyQty);
        }

        uint64_t calculateSellHypotheticalWorst() {
            return std::max(sellQty, sellQty-netPos);
        }

        public:

        FinancialInstrument():
        netPos(0),buyQty(0),sellQty(0),buyHypotheticalWorst(0),sellHypotheticalWorst(0) {

        }

        bool assessAddOrder(const std::unique_ptr<orders::Order> &order) {

        }

        void updateInstrumentMetrics() {

        }

        void handleTrade(const uint64_t &tradeQuantity, const char &side) {

        }

        bool assessModifyOrder(std::unique_ptr<orders::Order> &order, uint64_t newQty) {

        }

        void deleteOrder(std::unique_ptr<orders::Order> &order) {
            
        }


    };

}