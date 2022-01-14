#include <iostream>
#include <iomanip>
#include "debug.hpp"


void disassembleChunk(const chunk_t chunk_stack, std::string name){
    std::cout << name << std::endl;
    for(Iter iter=chunk_stack.begin(); iter != chunk_stack.end();){
        disassembleInstruction(&iter, &chunk_stack);
    }
}

void disassembleInstruction(Iter* iter, const chunk_t* chunk_stack){
    std::cout << std::setfill('0') << std::setw(4) << *iter - chunk_stack->begin();
    uint8_t instruction = **iter;
    switch (instruction){
        case OP_RETURN:
            simpleInstruction("OP_RETURN", iter);
            break;
        default:
            std::cout << " unknown operation code " << instruction << std::endl;
            ++(*iter);
            break;
    }
}

void simpleInstruction(std::string op_name, Iter* iter){
    std::cout << " " << op_name << std::endl;
    ++(*iter);
}