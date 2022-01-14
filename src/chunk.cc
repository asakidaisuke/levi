#include "chunk.hpp"


void Chunk::writeChunk(uint8_t bytecode){
    chunk_stack.push_back(bytecode);
}

const chunk_t Chunk::getChunk(){
    return chunk_stack;
}