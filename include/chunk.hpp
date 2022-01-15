#ifndef LEVI_CHUNK_H
#define LEVI_CHUNK_H

#include <vector>
#include <memory>
#include "value.hpp"

using chunk_array = std::vector<uint8_t>;
using line_array = std::vector<int>;
using Iter = chunk_array::const_iterator;

typedef enum {
    OP_CONSTANT,
    OP_RETURN,
} OpCode;


class Chunk{
    public:
        void writeChunk(uint8_t, int);
        void writeValue(value_t, int);
        chunk_array getChunk();
        value_t getValue(int);
        int getLine(int);

    private:
        line_array line_stack;
        chunk_array chunk_stack;
        Value value;
};


#endif