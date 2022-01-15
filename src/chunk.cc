#include "chunk.hpp"


void Chunk::writeChunk(uint8_t bytecode, int line){
    chunk_stack->push_back(bytecode);
    line_stack->push_back(line);
}

chunk_array* Chunk::getChunk(){
    return chunk_stack.get();
}

void Chunk::writeValue(value_t val, int line){
    uint8_t constant_index = value.addConstant(val);
    chunk_stack->push_back(constant_index);
    line_stack->push_back(line);
}

value_t Chunk::getValue(int index){
    return value.getElement(index);
}

int Chunk::getLine(int offset){
    return (*line_stack)[offset];
}