#ifndef LEVI_CHUNK_H
#define LEVI_CHUNK_H

#include <vector>
#include <memory>
#include "value.hpp"

using chunk_array = std::vector<uint8_t>;
using line_array = std::vector<int>;
using chunk_iter = chunk_array::const_iterator;

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN,
} OpCode;


class Chunk{
    public:
        void writeChunk(uint8_t, int);
        void writeValue(value_t, int);
        chunk_array* getChunk();
        value_t getValue(int);
        int getValueSize();
        int getLine(int);
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