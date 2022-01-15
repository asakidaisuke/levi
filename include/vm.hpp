#ifndef LEVI_VM_H
#define LEVI_VM_H

#include "chunk.hpp"
#include "value.hpp"

#define STACK_MAX 256

using stack_array = std::vector<value_t>;
using stack_iter = stack_array::iterator;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
}InterpretResult;

class VirtualMachine{
    public:
        InterpretResult interpret(Chunk* chunk);
        InterpretResult run();
        value_t stack_pop();
        void stack_push(value_t);
        VirtualMachine(): stack_ptr(0){
            stack_memory = std::make_unique<stack_array>(STACK_MAX);
            stack_ptr = stack_memory->begin();
        }
    private:
        chunk_iter ip;
        Chunk** chunk_ptr;
        std::unique_ptr<stack_array> stack_memory;
        stack_iter stack_ptr;
        // int stack_ptr;
};

#endif