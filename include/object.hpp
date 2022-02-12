#ifndef LEVI_OBJECT_H
#define LEVI_OBJECT_H

#include <iostream>
#include "common.hpp"
#include "value.hpp"
#include "chunk.hpp"
// #include "vm.hpp"

#define OBJ_TYPE(value)    (AS_OBJ(value)->type)
#define IS_CLOSURE(value)   Object::isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value)   Object::isObjType(value, OBJ_FUNCTION)
#define IS_STRING(value)   Object::isObjType(value, OBJ_STRING)
#define IS_NATIVE(value)   Object::isObjType(value, OBJ_NATIVE)

#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->strs)
#define AS_FUNCTION(value)  (((ObjFunction*)AS_OBJ(value)))
#define AS_NATIVE(value)  (((ObjNative*)AS_OBJ(value))->function)
#define AS_CLOSURE(value)  ((ObjClosure*)AS_OBJ(value))

using stack_array = std::vector<value_t>;
using stack_iter = stack_array::iterator;

typedef value_t (*NativeFn)(int argCount, stack_iter args);
struct ObjUpvalue;

enum ObjType{
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
};

struct Obj{
    ObjType type;
};

struct ObjNative{
    Obj obj;
    NativeFn function;
};

struct ObjFunction{
    Obj obj{OBJ_FUNCTION};
    int arity{0};
    int upvalueCount{0};
    std::unique_ptr<Chunk> chunk;
    ObjString* name;
};

struct ObjString{
    Obj obj;
    int length;
    std::string strs;
};

struct ObjClosure{
    ObjClosure(ObjFunction* arg_function)
    : upvalues(arg_function->upvalueCount, NULL)
    {
        function=arg_function;
        upvalueCount=function->upvalueCount;
    }
    Obj obj;
    ObjFunction* function;
    std::vector<ObjUpvalue*> upvalues;
    int upvalueCount;
};

struct ObjUpvalue{
    ObjUpvalue(value_t* slot){location=slot;}
    Obj obj;
    value_t* location{nullptr};
    ObjUpvalue* next{NULL};
    value_t closed{NIL_VAL};
};

class Object{
    public:
        static void getObjString(std::string strs, ObjString* objString){
            objString->obj = Obj{OBJ_STRING};
            objString->length = strs.size();
            objString->strs = strs;
        }

        static inline bool isObjType(value_t val, ObjType type){
            return IS_OBJ(val) && AS_OBJ(val)->type == type;
            }

        static void printObject(value_t val){
            switch(OBJ_TYPE(val)){
                case OBJ_STRING:
                    std::cout << AS_CSTRING(val) << std::endl;
                    break;
                case OBJ_FUNCTION:
                    std::cout << "Function: " << AS_FUNCTION(val) << std::endl;
                    break;
                case OBJ_NATIVE:
                    std::cout << "<native fn>"<< std::endl;
                    break;
                case OBJ_CLOSURE:
                    std::cout << "<closuer>"<< std::endl;
                    break;
                case OBJ_UPVALUE:
                    std::cout << "upvalue" << std::endl;
                    break;
            }
        }
};

# endif