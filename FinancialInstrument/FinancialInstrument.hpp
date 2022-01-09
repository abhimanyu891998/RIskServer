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


/**
 * Important part of the application, stores all the logic to identify if the order should be accepted or rejected.
 * Updates state of the system accordingly, handles trades and deletion of orders as well.
**/
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
            return std::max(buyQty, netPos+buyQty); //Based on the formula given in the problem statement
        }

        uint64_t calculateSellHypotheticalWorst() {
            return std::max(sellQty, sellQty-netPos); //Based on the formula given in the problem statement
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


        /**
         * Check the new order and decides whether to accept it or not based on hypothetical worst position
        **/
        bool assessAddOrder(const std::shared_ptr<orders::Order> &order) {
            cout<<"Assessing new order with order ID: "<<order->orderId<<std::endl;
            cout<<"Threshold values for BUY and SELL respectively are: "<<BUY_THRESHOLD<<" "<<SELL_THRESHOLD<<std::endl;
            switch(order->side) {

                case 'B' : { 
                          
                          buyQty+=order->quantity;
                          uint64_t currentBuyHypotheticalWorst = calculateBuyHypotheticalWorst();
                          cout<<"Side for this order is BUY and currentBuyHypotheticalWorst is: "<<currentBuyHypotheticalWorst<<std::endl;
                          if(currentBuyHypotheticalWorst > BUY_THRESHOLD) {
                              buyQty-=order->quantity; //Revert state of the system
                              currentBuyHypotheticalWorst = calculateBuyHypotheticalWorst();
                              setBuyHypotheticalWorst(currentBuyHypotheticalWorst);
                              return false;
                          }
                            setBuyHypotheticalWorst(currentBuyHypotheticalWorst);
                            break;
                          }

                case 'S' : { sellQty+=order->quantity;
                          uint64_t currentSellHypotheticalWorst = calculateSellHypotheticalWorst();
                          cout<<"Side for this order is SELL and currentBuyHypotheticalWorst is: "<<currentSellHypotheticalWorst<<std::endl;
                          if(currentSellHypotheticalWorst > SELL_THRESHOLD) {
                              sellQty-=order->quantity; //Revert state of the system
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

        //Updates netPos and recalculates HypotheticalWorstPositions
        void handleTrade(const uint64_t &tradeQuantity, const char &side) {
            netPos += tradeQuantity;
            setBuyHypotheticalWorst(calculateBuyHypotheticalWorst());
            setSellHypotheticalWorst(calculateSellHypotheticalWorst());
        }

        /**
         * Similar to add new order, it checks if it is acceptable to modify the order.
        **/
        bool assessModifyOrder(std::shared_ptr<orders::Order> &order, uint64_t newQty) {
            cout<<"Assessing new order with order ID: "<<order->orderId<<std::endl;
            cout<<"Threshold values for BUY and SELL respectively are: "<<BUY_THRESHOLD<<" "<<SELL_THRESHOLD<<std::endl;           
            switch(order->side) {

                case 'B': { buyQty-=order->quantity;
                          buyQty+=newQty;
                          uint64_t currentBuyHypotheticalWorst = calculateBuyHypotheticalWorst();
                          cout<<"Side for this order is BUY and currentBuyHypotheticalWorst is: "<<currentBuyHypotheticalWorst<<std::endl;
                          if(currentBuyHypotheticalWorst > BUY_THRESHOLD) {
                              buyQty-=newQty;
                              buyQty+=order->quantity; //Revert state
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
                          cout<<"Side for this order is SELL and currentBuyHypotheticalWorst is: "<<currentSellHypotheticalWorst<<std::endl;
                          if(currentSellHypotheticalWorst > SELL_THRESHOLD) {
                              sellQty-=newQty;
                              sellQty+=order->quantity; //Revert state
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

            return true;
        }

        /**
         * Removes the order and updates the state of this instrument
         **/
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