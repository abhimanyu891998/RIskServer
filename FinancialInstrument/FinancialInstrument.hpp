#pragma once

#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<unordered_map>
#include "../messageStructures.hpp"
#include "../Order/Order.hpp"
#include "../Config/config.hpp"

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
        uint64_t BUY_THRESHOLD;
        uint64_t SELL_THRESHOLD;


        uint64_t calculateBuyHypotheticalWorst() {
            return std::max(buyQty, netPos+buyQty);
        }

        uint64_t calculateSellHypotheticalWorst() {
            return std::max(sellQty, sellQty-netPos);
        }

        void setBuyHypotheticalWorst(const uint64_t &value) {
            buyHypotheticalWorst = value;
        }

        void setSellHypotheticalWorst(const uint64_t &value) {
            sellHypotheticalWorst = value;
        }

        public:

        FinancialInstrument(const configuration::Config &config):
            netPos(0),buyQty(0),sellQty(0),buyHypotheticalWorst(0),sellHypotheticalWorst(0),BUY_THRESHOLD(config.BUY_THRESHOLD),SELL_THRESHOLD(config.SELL_THRESHOLD){

        }

        bool assessAddOrder(const std::shared_ptr<orders::Order> &order) {

            switch(order->side) {

                case 'B' : { 
                          
                          buyQty+=order->quantity;
                          uint64_t currentBuyHypotheticalWorst = calculateBuyHypotheticalWorst();
                          if(currentBuyHypotheticalWorst > BUY_THRESHOLD) {
                              buyQty-=order->quantity;
                              currentBuyHypotheticalWorst = calculateBuyHypotheticalWorst();
                              setBuyHypotheticalWorst(currentBuyHypotheticalWorst);
                              return false;
                          }
                            setBuyHypotheticalWorst(currentBuyHypotheticalWorst);
                            break;
                          }

                case 'S' : { sellQty+=order->quantity;
                          uint64_t currentSellHypotheticalWorst = calculateSellHypotheticalWorst();
                          if(currentSellHypotheticalWorst > SELL_THRESHOLD) {
                              sellQty-=order->quantity;
                              currentSellHypotheticalWorst = calculateSellHypotheticalWorst();
                              setSellHypotheticalWorst(currentSellHypotheticalWorst);
                              return false;
                          }
                          setSellHypotheticalWorst(currentSellHypotheticalWorst);
                          break;
                          }

                default : { std::cerr<<"Invalid order side supplied";
                         return false;
                         break;
                         }
            }

            return true;
        }


        void handleTrade(const uint64_t &tradeQuantity, const char &side) {
            netPos += tradeQuantity;
            setBuyHypotheticalWorst(calculateBuyHypotheticalWorst());
            setSellHypotheticalWorst(calculateSellHypotheticalWorst());
        }

        bool assessModifyOrder(std::shared_ptr<orders::Order> &order, uint64_t newQty) {
            
            switch(order->side) {

                case 'B': { buyQty-=order->quantity;
                          buyQty+=newQty;
                          uint64_t currentBuyHypotheticalWorst = calculateBuyHypotheticalWorst();
                          if(currentBuyHypotheticalWorst > BUY_THRESHOLD) {
                              buyQty-=newQty;
                              buyQty+=order->quantity;
                              currentBuyHypotheticalWorst = calculateBuyHypotheticalWorst();
                              setBuyHypotheticalWorst(currentBuyHypotheticalWorst);
                              return false;
                            }
                            setBuyHypotheticalWorst(currentBuyHypotheticalWorst);
                            break;
                            }

                case 'S': { sellQty-=order->quantity;
                          sellQty+=newQty;
                          uint64_t currentSellHypotheticalWorst = calculateSellHypotheticalWorst();
                          if(currentSellHypotheticalWorst > SELL_THRESHOLD) {
                              sellQty-=newQty;
                              sellQty+=order->quantity;
                              currentSellHypotheticalWorst = calculateSellHypotheticalWorst();
                              setSellHypotheticalWorst(currentSellHypotheticalWorst);
                              return false;
                          }
                          setSellHypotheticalWorst(currentSellHypotheticalWorst);
                          break;
                          }

                default: { std::cerr<<"Invalid order side supplied";
                         return false;
                         break;
                         }
            }
        }

        void deleteOrder(std::shared_ptr<orders::Order> &order) {

            switch(order->side) {

                case 'B': { buyQty-=order->quantity;
                          setBuyHypotheticalWorst(calculateBuyHypotheticalWorst());
                          break;
                          }

                case 'S': { sellQty-=order->quantity;
                          setSellHypotheticalWorst(calculateSellHypotheticalWorst());
                          break;
                          }


                default: { std::cerr<<"Invalid order side supplied";
                         break;
                         }                          

            }

            return;
        }

        uint64_t getBuyHypotheticalWorst() {
            return buyHypotheticalWorst;
        }

        uint64_t getSellHypotheticalWorst() {
            return sellHypotheticalWorst;
        }

    };

}