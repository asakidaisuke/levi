#ifndef LEVI_OBJECT_H
#define LEVI_OBJECT_H

#include <iostream>
#include <unordered_map>
#include <functional>
#include "common.hpp"
#include "value.hpp"
#include "chunk.hpp"
// #include "vm.hpp"

#define OBJ_TYPE(value)    (AS_OBJ(value)->type)
#define IS_CLOSURE(value)   Object::isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value)   Object::isObjType(value, OBJ_FUNCTION)
#define IS_STRING(value)   Object::isObjType(value, OBJ_STRING)
#define IS_NATIVE(value)   Object::isObjType(value, OBJ_NATIVE)
#define IS_CLASS(value)   Object::isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value) Object::isObjType(value, OBJ_INSTANCE)
#define IS_BOUND_METHOD(value) Object::isObjType(value, OBJ_BOUND_METHOD)

#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->strs)
#define AS_FUNCTION(value)  (((ObjFunction*)AS_OBJ(value)))
#define AS_NATIVE(value)  (((ObjNative*)AS_OBJ(value))->function)
#define AS_CLOSURE(value)  ((ObjClosure*)AS_OBJ(value))
#define AS_CLASS(value)  ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))

using stack_array = std::vector<value_t>;
using stack_iter = stack_array::iterator;
using NativeFn = std::function<value_t(int, stack_iter)>;

struct ObjUpvalue;

enum ObjType{
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
};

struct Obj{
    ObjType type;
};

struct ObjNative{
    ObjNative(NativeFn fun){function=fun;}
    Obj obj{OBJ_NATIVE};
    NativeFn function;
};

struct ObjFunction{
    Obj obj{OBJ_FUNCTION};
    int arity{0};
    int upvalueCount{0};
    std::unique_ptr<Chunk> chunk;
    std::string name{"main"};
};

struct ObjString{
    Obj obj{OBJ_STRING};
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
    Obj obj{OBJ_CLOSURE};
    ObjFunction* function;
    std::vector<ObjUpvalue*> upvalues;
    int upvalueCount;
};

struct ObjUpvalue{
    ObjUpvalue(value_t* slot){location=slot;}
    Obj obj{OBJ_UPVALUE};
    value_t* location{nullptr};
    ObjUpvalue* next{NULL};
    value_t closed{NIL_VAL};
};

struct ObjClass{
    ObjClass(ObjString* obj_name) {name=obj_name;}
    Obj obj{OBJ_CLASS};
    ObjString* name;
    std::unordered_map<std::string, value_t> methods;
};

struct ObjInstance{
    ObjInstance(ObjClass* arg_klass){klass=arg_klass;}
    Obj obj{OBJ_INSTANCE};
    ObjClass* klass;
    std::unordered_map<std::string, value_t> fields;
};

struct ObjBoundMethod{
    ObjBoundMethod(value_t arg_receiver, ObjClosure* arg_method)
    {
        receiver = arg_receiver;
        method = arg_method;
    }
    Obj obj{OBJ_BOUND_METHOD};
    value_t receiver;
    ObjClosure* method;
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
                    std::cout << AS_CSTRING(val);
                    break;
                case OBJ_FUNCTION:
                    std::cout << "Function: " << AS_FUNCTION(val)->name;
                    break;
                case OBJ_NATIVE:
                    std::cout << "<native fn>";
                    break;
                case OBJ_CLOSURE:
                    std::cout << "<closuer>";
                    break;
                case OBJ_UPVALUE:
                    std::cout << "upvalue";
                    break;
                case OBJ_CLASS:
                    std::cout << "class";
                    break;
                case OBJ_INSTANCE:
                    std::cout << "instance";
                    break;
            }
        }
};

# endif