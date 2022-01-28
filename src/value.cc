#include <iostream>
#include "value.hpp"
#include "object.hpp"


uint8_t Value::addConstant(value_t value){
    value_stack.push_back(value);
    return value_stack.size() - 1;
}

value_t Value::getElement(int index){
    return value_stack[index];
}

void Value::printValue(value_t val){
    switch(val.type){
        case VAL_BOOL:{
            std::string is_ = AS_BOOL(val) ? "true" : "false";
            std::cout << is_ << std::endl;
            break;}
        case VAL_NIL:{
            std::cout << "nil" << std::endl;
            break;}
        case VAL_NUMBER:{
            std::cout << AS_NUMBER(val) << std::endl;
            break;}
        case VAL_OBJ:{
            Object::printObject(val);
            break;}
    }
}

bool Value::valuesEqual(value_t a, value_t b){
    if(a.type != b.type) return false;
    switch(a.type){
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:
            return true;
        case VAL_NUMBER:
            return AS_NUMBER(a) = AS_NUMBER(b);
        case VAL_OBJ: {
            ObjString* aString = AS_STRING(a);
            ObjString* bString = AS_STRING(b);
            return aString->strs == bString->strs;
            }
        default:
            return false;
    }
}