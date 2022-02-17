#ifndef LEVI_CHUNK_H
#define LEVI_CHUNK_H

#include <vector>
#include <memory>
#include "value.hpp"

using chunk_array = std::vector<uint8_t>;
using line_array = std::vector<int>;
using chunk_iter = chunk_array::const_iterator;

enum OpCode {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER_INVOKE,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_METHOD,
    OP_INHERIT,
    OP_GET_SUPER,
};

class Chunk{
    public:
        void writeChunk(uint8_t, int);
        void writeValue(value_t, int);
        chunk_array* getChunk();
        value_t getValue(int);
        int getValueSize();
        int getLine(int);
        uint8_t addConstantToValue(value_t);
        Chunk(){
            chunk_stack = std::make_unique<chunk_array>();
            line_stack = std::make_unique<line_array>();
        }

    private:
        std::unique_ptr<line_array> line_stack;
        std::unique_ptr<chunk_array> chunk_stack;
        Value value;
};


#endif