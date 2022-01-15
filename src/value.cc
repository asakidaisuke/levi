#include "value.hpp"


uint8_t Value::addConstant(value_t value){
    value_stack.push_back(value);
    return value_stack.size() - 1;
}

value_t Value::getElement(int index){
    return value_stack[index];
}