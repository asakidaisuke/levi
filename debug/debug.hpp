#ifndef LEVI_DEBUG_H
#define LEVI_DEBUG_H

#include "chunk.hpp"

void disassembleChunk(std::string, Chunk*);
void disassembleInstruction(Iter*, const chunk_array*, Chunk*);
void simpleInstruction(std::string, Iter*);
void constantInstruction(std::string, Iter*, Chunk*);


#endif