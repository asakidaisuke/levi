#ifndef LEVI_VALUE_H
#define LEVI_VALUE_H

#include "common.hpp"
#include "vector"

using value_t = double;
using ValueArray = std::vector<value_t>;


class Value{
    public:
        uint8_t addConstant(value_t value);
        value_t getElement(int index);
        int getValueStackSize(){
            return value_stack.size();
        }
    private:
        ValueArray value_stack;
};


#endif