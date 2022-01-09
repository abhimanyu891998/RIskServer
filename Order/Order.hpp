#pragma once

#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<unordered_map>
#include "../messageStructures.hpp"

using std::cout;
using std::cin;
using std::vector;
using std::unordered_map;

/**
 * Simple order struct to store order specific information
**/
namespace orders {

    struct Order {

        uint64_t orderId;
        uint64_t instrumentId;
        uint64_t quantity;
        uint64_t price;
        char side;

        Order(uint64_t orderId, uint64_t instrumentId, uint64_t quantity, uint64_t price, char side):
        orderId(orderId),instrumentId(instrumentId),quantity(quantity),price(price),side(side){

        }

    };

}