#include <iostream>
#include "vm.hpp"
#include "common.hpp"
#include "debug.hpp"

#define BINARY_OP(op) do{\
    double b = stack_pop();\
    double a = stack_pop();\
    stack_push(a op b);\
} while(false)

void VirtualMachine::stack_push(value_t val){
    *stack_ptr = val;
    (stack_ptr)++;
}

value_t VirtualMachine::stack_pop(){
    stack_ptr--;
    return *stack_ptr;
}

InterpretResult VirtualMachine::interpret(Chunk* chunk){
    chunk_ptr = &chunk;
    ip = chunk->getChunk()->begin();
    return run();
}

InterpretResult VirtualMachine::run(){
    auto read_byte = [](chunk_iter* ip){chunk_iter ret_ip = *ip; (*ip)++; return *ret_ip;};
    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            std::cout << "             " << std::endl;
            for(stack_iter slot = stack_memory->begin(); slot != stack_ptr; slot++){
                std::cout << "[" << *slot << "]" << std::endl;
            }
            // disassembleInstruction(&ip, *chunk_ptr);
        #endif
        
        uint8_t instruction;
        switch (instruction = read_byte(&ip)){
            case OP_CONSTANT:{
                // auto read_const = [](Iter* ip){return chunk->getValue(read_byte(&ip));};
                value_t constant = (*chunk_ptr)->getValue(read_byte(&ip));
                stack_push(constant);
                break;
            }
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
            case OP_NEGATE:{
                stack_push(-stack_pop());
                break;
            }
            case OP_RETURN:{
                std::cout << stack_pop() << std::endl;
                return INTERPRET_OK;
            }
        }
    }
}


