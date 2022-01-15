#ifndef LEVI_DEBUG_H
#define LEVI_DEBUG_H

#include "chunk.hpp"

void disassembleChunk(std::string, Chunk*);
void disassembleInstruction(chunk_iter*, Chunk*);
void simpleInstruction(std::string, chunk_iter*);
void constantInstruction(std::string, chunk_iter*, Chunk*);


#endif