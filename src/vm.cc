#include <iostream>
#include "vm.hpp"
#include "common.hpp"
#include "debug.hpp"
#include "compiler.hpp"


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
            }
            // disassembleInstruction(&ip, *chunk_ptr);
        #endif
        
        uint8_t instruction;
        switch (instruction = read_byte(&ip)){
            case OP_CONSTANT:{
                // auto read_const = [](Iter* ip){return chunk->getValue(read_byte(&ip));};
                value_t constant = chunk->getValue(read_byte(&ip));
                stack_push(constant);
                break;
            }
            case OP_NIL: stack_push(NIL_VAL); break;
            case OP_TRUE: stack_push(BOOL_VAL(true)); break;
            case OP_FALSE: stack_push(BOOL_VAL(false)); break;
            case OP_EQUAL:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                bool flag = Value::valuesEqual(a, b);
                stack_push(BOOL_VAL(flag));
                break;
            }
            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: BINARY_OP(NUMBER_VAL, +); break;
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
            case OP_RETURN:{
                value_t ret = stack_pop();
                Value::printValue(ret);
                return INTERPRET_OK;
            }
        }
    }
}


