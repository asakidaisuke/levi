#ifndef LEVI_VM_H
#define LEVI_VM_H

#include <unordered_map>
#include "chunk.hpp"
#include "value.hpp"
#include "object.hpp"
#include "compiler.hpp"
#include "common.hpp"
#include "debug.hpp"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

using stack_array = std::vector<value_t>;
using stack_iter = stack_array::iterator;

struct CallFrame{
    ObjClosure* closure;
    chunk_iter ip;
    stack_iter slots;
};

enum InterpretResult{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VirtualMachine{
    public:
        InterpretResult interpret(Chunk* chunk);
        InterpretResult interpret(std::string source);
        InterpretResult run();
        void stack_push(value_t);
        VirtualMachine(): stack_ptr(0){
            stack_memory = std::make_unique<stack_array>(STACK_MAX);
            stack_ptr = stack_memory->begin();
        }
    private:
        chunk_iter ip;
        std::unique_ptr<Chunk> chunk;
        std::unique_ptr<stack_array> stack_memory;
        stack_iter stack_ptr;
        value_t stack_pop();
        value_t peek(int);
        bool isFalsey(value_t val);
        void runtimeError(std::string format);
        void concatenate();
        bool call(ObjClosure*, int);
        bool callValue(value_t callee, int argCount);
        Obj* object;
        std::unordered_map<std::string, value_t> globals_table;
        CallFrame frames[FRAMES_MAX];
        int frameCount{0};
};

#endif