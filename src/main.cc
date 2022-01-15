#include <iostream>
#include "chunk.hpp"
#include "debug.hpp"

int main(int, char**) {
    Chunk chunk;
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeValue(1.2, 123);
    chunk.writeChunk(OP_RETURN, 123);
    disassembleChunk("test_chunk", &chunk);
}
