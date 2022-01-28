#include <iostream>
#include <iomanip>
#include "debug.hpp"


void disassembleChunk(std::string name, Chunk* chunk){
    std::cout << name << std::endl;
    chunk_array* chunk_stack = chunk->getChunk();
    for(chunk_iter iter=chunk_stack->begin(); iter != chunk_stack->end();){
        disassembleInstruction(&iter, chunk);
    }
}

void disassembleInstruction(chunk_iter* iter, Chunk* chunk){
    int offset = *iter - chunk->getChunk()->begin();
    std::cout << std::setfill('0') << std::setw(4) << offset;
    if (offset > 0 && chunk->getLine(offset) == chunk->getLine(offset-1)){
        std::cout << "  |  ";
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
        case OP_NIL:
            simpleInstruction("OP_NIL", iter);
            break;
        case OP_TRUE:
            simpleInstruction("OP_TRUE", iter);
            break;
        case OP_FALSE:
            simpleInstruction("OP_FALSE", iter);
            break;
        case OP_POP:
            simpleInstruction("OP_POP", iter);
            break;
        case OP_GET_GLOBAL:
            simpleInstruction("OP_GET_GLOBAL", iter);
            break;
        case OP_DEFINE_GLOBAL:
            constantInstruction("OP_DEFINE_GLOBAL", iter, chunk);
            break;
        case OP_SET_GLOBAL:
            simpleInstruction("OP_SET_GLOBAL", iter);
            break;
        case OP_EQUAL:
            simpleInstruction("OP_EQUAL", iter);
            break;
        case OP_GREATER:
            simpleInstruction("OP_GREATER", iter);
            break;
        case OP_LESS:
            simpleInstruction("OP_LESS", iter);
            break;
        case OP_ADD: 
            simpleInstruction("OP_ADD", iter);
            break;
        case OP_SUBTRACT: 
            simpleInstruction("OP_SUBTRACT", iter);
            break;
        case OP_MULTIPLY: 
            simpleInstruction("OP_MULTIPLY", iter);
            break;
        case OP_DIVIDE: 
            simpleInstruction("OP_DIVIDE", iter);
            break;
        case OP_NOT:
            simpleInstruction("OP_NOT", iter);
            break;
        case OP_NEGATE:
            simpleInstruction("OP_NEGATE", iter);
            break;
        case OP_PRINT:
            simpleInstruction("OP_PRINT", iter);
        default:
            std::cout << "unknown operation code " << instruction << std::endl;
            ++(*iter);
            break;
    }
}

void simpleInstruction(std::string op_name, chunk_iter* iter){
    std::cout << " " << op_name << std::endl;
    ++(*iter);
}

void constantInstruction(std::string op_name, chunk_iter* iter, Chunk* chunk){
    std::cout << " " << op_name;
    uint8_t offset = *(++(*iter));
    value_t val = chunk->getValue(offset);
    std::cout << " " << (long)offset << " ";
    Value::printValue(val);
    ++(*iter);
}