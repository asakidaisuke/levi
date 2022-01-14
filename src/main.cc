#include <iostream>
#include "chunk.hpp"
#include "debug.hpp"

int main(int, char**) {
    Chunk chunk;
    chunk.writeChunk(OP_RETURN);
    disassembleChunk(chunk.getChunk(), "test_chunk");
}
