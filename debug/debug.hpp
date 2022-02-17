#ifndef LEVI_DEBUG_H
#define LEVI_DEBUG_H

#include "chunk.hpp"

void disassembleChunk(std::string, Chunk*);
void disassembleInstruction(chunk_iter*, Chunk*);
void simpleInstruction(std::string, chunk_iter*);
void constantInstruction(std::string, chunk_iter*, Chunk*);
void byteInstruction(std::string name, chunk_iter* iter, Chunk* chunk);
void jumpInstruction(std::string name, int sign, Chunk* chunk,  chunk_iter* iter);
void invokeInstruction(std::string op_name, chunk_iter *iter, Chunk *chunk);
std::string get_op_code(uint8_t code);


#endif