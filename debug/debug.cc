#include <iostream>
#include <iomanip>
#include "debug.hpp"


void disassembleChunk(std::string name, Chunk* chunk){
    std::cout << name << std::endl;
    const chunk_array chunk_stack = chunk->getChunk();
    for(Iter iter=chunk_stack.begin(); iter != chunk_stack.end();){
        disassembleInstruction(&iter, &chunk_stack, chunk);
    }
}

void disassembleInstruction(Iter* iter, const chunk_array* chunk_stack, Chunk* chunk){
    int offset = *iter - chunk_stack->begin();
    std::cout << std::setfill('0') << std::setw(4) << offset;
    if (offset > 0 && chunk->getLine(offset) == chunk->getLine(offset-1)){
        std::cout << "     |  ";
    }else{
        std::cout << " " << std::setfill('0') << std::setw(4) << chunk->getLine(offset);
    }

    uint8_t instruction = **iter;
    switch (instruction){
        case OP_RETURN:
            simpleInstruction("OP_RETURN", iter);
            break;
        case OP_CONSTANT:
            constantInstruction("OP_CONSTANT", iter, chunk);
            break;
        default:
            std::cout << "unknown operation code " << instruction << std::endl;
            ++(*iter);
            break;
    }
}

void simpleInstruction(std::string op_name, Iter* iter){
    std::cout << " " << op_name << std::endl;
    ++(*iter);
}

void constantInstruction(std::string op_name, Iter* iter, Chunk* chunk){
    std::cout << " " << op_name;
    uint8_t offset = *(++(*iter));
    value_t val = chunk->getValue(offset);
    std::cout << " " << (long)offset << " " << val << std::endl;
    ++(*iter);
}