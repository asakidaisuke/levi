#ifndef LEVI_DEBUG_H
#define LEVI_DEBUG_H

#include "chunk.hpp"

using Iter = chunk_t::const_iterator;

void disassembleChunk(const chunk_t, std::string);
void disassembleInstruction(Iter*, const chunk_t*);
void simpleInstruction(std::string, Iter*);


#endif