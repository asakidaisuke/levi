#include <iostream>
#include "vm.hpp"
#include "common.hpp"
#include "debug.hpp"
#include "compiler.hpp"
#include "object.hpp"


#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(stack_pop()); \
      double a = AS_NUMBER(stack_pop()); \
      stack_push(valueType(a op b)); \
    } while (false)

void VirtualMachine::stack_push(value_t val){
    *stack_ptr = val;
    (stack_ptr)++;
}

value_t VirtualMachine::stack_pop(){
    stack_ptr--;
    return *stack_ptr;
}

value_t VirtualMachine::peek(int offset){
    return *(stack_ptr-offset-1);
}

bool VirtualMachine::isFalsey(value_t val){
    return IS_NIL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}

void VirtualMachine::concatenate(){
    ObjString* b = AS_STRING(stack_pop());
    ObjString* a = AS_STRING(stack_pop());
    int con_length = a->length + b->length;
    std::string con_strs = a->strs + b->strs;
    ObjString* c = new ObjString{OBJ_STRING, con_length, con_strs};
    stack_push(OBJ_VAL(c));
}

InterpretResult VirtualMachine::interpret(std::string source){
    Compiler compiler(source);
    if(!compiler.compile(source)){
        return INTERPRET_COMPILE_ERROR;
    }
    chunk = compiler.get_chunk();
    ip = chunk->getChunk()->begin();  // return first element iterator

    InterpretResult result = run();
    return result;
}

void VirtualMachine::runtimeError(std::string format){
    size_t offset = ip - chunk->getChunk()->begin();
    int line = chunk->getLine(offset);
    std::cout << "[line " << line << "] Error";
}

InterpretResult VirtualMachine::run(){
    auto read_byte = [](chunk_iter* ip){chunk_iter ret_ip = *ip; (*ip)++; return *ret_ip;};
    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            std::cout << "             " << std::endl;
            for(stack_iter slot = stack_memory->begin(); slot != stack_ptr; slot++){
                if (slot->type == VAL_BOOL)
                    std::cout << "[" << AS_BOOL(*slot) << "]" << std::endl;
                if (slot->type == VAL_NUMBER)
                    std::cout << "[" << AS_NUMBER(*slot) << "]" << std::endl;
                if (IS_STRING(*slot))
                    std::cout << "[" << AS_CSTRING(*slot) << "]" << std::endl;
            }
            // disassembleInstruction(&ip, *chunk_ptr);
        #endif
        
        uint8_t instruction;
        switch (instruction = read_byte(&ip)){
            case OP_CONSTANT:{
                value_t constant = chunk->getValue(read_byte(&ip));
                stack_push(constant);
                break;
            }
            case OP_NIL: stack_push(NIL_VAL); break;
            case OP_TRUE: stack_push(BOOL_VAL(true)); break;
            case OP_FALSE: stack_push(BOOL_VAL(false)); break;
            case OP_POP: stack_pop(); break;
            case OP_GET_LOCAL:{
                uint8_t slot = read_byte(&ip);
                stack_push((*stack_memory)[slot]);
                break;
            }
            case OP_SET_LOCAL:{
                uint8_t slot = read_byte(&ip);
                (*stack_memory)[slot] = peek(0);
                break;
            }
            case OP_GET_GLOBAL:{
                ObjString* name = AS_STRING(chunk->getValue(read_byte(&ip)));
                value_t val;
                if (globals_table.find(name->strs) == globals_table.end()){
                    std::string format = "Undifined variable " + name->strs + ".";
                    runtimeError(format);
                    return INTERPRET_RUNTIME_ERROR;
                }else{
                    val = globals_table[name->strs];
                }
                stack_push(val);
                break;
            }
            case OP_DEFINE_GLOBAL:{
                ObjString* name = AS_STRING(chunk->getValue(read_byte(&ip)));
                globals_table[name->strs] = peek(0);
                stack_pop();
                break;
            }
            case OP_SET_GLOBAL:{
                ObjString* name = AS_STRING(chunk->getValue(read_byte(&ip)));
                if (globals_table.find(name->strs) == globals_table.end()){
                    std::string format = "Undifined variable " + name->strs + ".";
                    runtimeError(format);
                    return INTERPRET_RUNTIME_ERROR;
                }else{
                    globals_table[name->strs] = peek(0);
                }
                break;
            }
            case OP_EQUAL:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                bool flag = Value::valuesEqual(a, b);
                stack_push(BOOL_VAL(flag));
                break;
            }
            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
                if(IS_STRING(peek(0)) && IS_STRING(peek(1))){
                    concatenate();
                }else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
                    BINARY_OP(NUMBER_VAL, +);
                }else{
                    runtimeError("Operands must be two number or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT: stack_push(BOOL_VAL(isFalsey(stack_pop()))); break;
            case OP_NEGATE:{
                if (!IS_NUMBER(peek(0))){
                    runtimeError("Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                stack_push(NUMBER_VAL(- AS_NUMBER(stack_pop())));
                break;
            }
            case OP_PRINT:{
                Value::printValue(stack_pop());
                std::cout << std::endl;
                break;
            }
            case OP_RETURN:{
                // value_t ret = stack_pop();
                // Value::printValue(ret);
                return INTERPRET_OK;
            }
        }
    }
}


