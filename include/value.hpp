#ifndef LEVI_VALUE_H
#define LEVI_VALUE_H

#include <memory>
#include "common.hpp"
#include "vector"

enum ValueType{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
};

struct Obj;
struct ObjString;

struct value_t{
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj; //ここ
    } as;
};

using ValueArray = std::vector<value_t>;

#define IS_BOOL(value)    ((value).type  == VAL_BOOL)
#define IS_NIL(value)     ((value).type  == VAL_NIL)
#define IS_NUMBER(value)  ((value).type  == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type  == VAL_OBJ)

#define AS_OBJ(value)     ((value).as.obj)
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define OBJ_VAL(object)   ((value_t){VAL_OBJ, {.obj=(Obj*)object}})
#define BOOL_VAL(value)   ((value_t){VAL_BOOL, {.boolean=value}})
#define NIL_VAL           ((value_t){VAL_NIL, {.number=0}})
#define NUMBER_VAL(value) ((value_t){VAL_NUMBER, {.number=value}})


class Value{
    public:
        uint8_t addConstant(value_t value);
        value_t getElement(int index);
        static bool valuesEqual(value_t, value_t);
        int getValueStackSize(){
            return value_stack.size();
        }
        static void printValue(value_t);
        template< typename T>
        static inline value_t obj_val(std::unique_ptr<T> object){
            return value_t{VAL_OBJ, .obj=std::move(static_cast<std::unique_ptr<Obj>>(object))};
        }
    private:
        ValueArray value_stack;
};


#endif