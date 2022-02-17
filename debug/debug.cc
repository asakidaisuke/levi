#include <iostream>
#include <iomanip>
#include "debug.hpp"
#include "object.hpp"


void disassembleChunk(std::string name, Chunk* chunk){
    std::cout << name << std::endl;
    chunk_array* chunk_stack = chunk->getChunk();
    for(chunk_iter iter=chunk_stack->begin(); iter != chunk_stack->end();){
        disassembleInstruction(&iter, chunk);
    }
    std::cout << std::endl;
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
        case OP_GET_LOCAL:
            byteInstruction("OP_GET_LOCAL", iter, chunk);
            break;
        case OP_SET_LOCAL:
            byteInstruction("OP_SET_LOCAL", iter, chunk);
            break;
        case OP_GET_GLOBAL:
            constantInstruction("OP_GET_GLOBAL", iter, chunk);
            break;
        case OP_DEFINE_GLOBAL:
            constantInstruction("OP_DEFINE_GLOBAL", iter, chunk);
            break;
        case OP_SET_GLOBAL:
            constantInstruction("OP_SET_GLOBAL", iter, chunk);
            break;
        case OP_GET_UPVALUE:
            byteInstruction("OP_GET_UPVALUE", iter, chunk);
            break;
        case OP_SET_UPVALUE:
            byteInstruction("OP_SET_UPVALUE", iter, chunk);
            break;
        case OP_GET_PROPERTY:
            constantInstruction("OP_GET_PROPERTY", iter, chunk);
            break;
        case OP_SET_PROPERTY:
            constantInstruction("OP_SET_PROPERTY", iter, chunk);
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
            break;
        case OP_JUMP:
            jumpInstruction("OP_JUMP", 1, chunk, iter);
            break;
        case OP_JUMP_IF_FALSE:
            jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, iter);
            break;
        case OP_LOOP:
            jumpInstruction("OP_LOOP", -1, chunk, iter);
            break;
        case OP_CALL:
            byteInstruction("OP_CALL", iter, chunk);
            break;
        case OP_INVOKE:
            invokeInstruction("OP_INVOKE", iter, chunk);
            break;
        case OP_CLASS:
            constantInstruction("OP_CLASS", iter, chunk);
            break;
        case OP_CLOSURE: {
            ++(*iter);
            std::cout << " OP_CLOSURE" << " " << (int8_t)**iter << " ";
            value_t constant = chunk->getValue(**iter);
            Value::printValue(constant);
            ObjFunction* function = AS_FUNCTION(constant);
            for (int j = 0; j < function->upvalueCount; j++) {
                ++(*iter);
                int isLocal = **iter;
                ++(*iter);
                int index = **iter;
                // print if it's local.
                std::cout << " |  " << isLocal ? "local" : "upvalue";
                // print index of upvalue
                std::cout << " " << index;
            } 
            std::cout << std::endl;
            ++(*iter);
            break;
        }
        case OP_METHOD:
            constantInstruction("OP_METHOD", iter, chunk);
            break;
        case OP_INHERIT:
            simpleInstruction("OP_INHERIT", iter);
            break;
        case OP_GET_SUPER:
            constantInstruction("OP_GET_SUPER", iter, chunk);
            break;
        case OP_SUPER_INVOKE:
            invokeInstruction("OP_SUPER_INVOKE", iter, chunk);
            break;
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

void byteInstruction(std::string op_name, chunk_iter* iter, Chunk* chunk){
    uint8_t offset = *(++(*iter));
    std::cout << " " << op_name << " " << (int)offset << std::endl;
    ++(*iter);
}

void constantInstruction(std::string op_name, chunk_iter* iter, Chunk* chunk){
    std::cout << " " << op_name;
    uint8_t offset = *(++(*iter));
    if (offset < chunk->getValueSize()){
        value_t val = chunk->getValue(offset);
        std::cout << " " << (long)offset << " ";
        Value::printValue(val);
        std::cout << std::endl;
    }else{
        std::cout << std::endl;
    }
    ++(*iter);
}

void jumpInstruction(std::string op_name, int sign,
                           Chunk* chunk, chunk_iter* iter) {
  uint16_t jump = (uint16_t)(*((*iter)+1) << 8);
  jump |= *((*iter) + 2);
  std::cout << " " << op_name << " " << sign*(uint16_t)jump << std::endl;
  *iter += 3;
}

void invokeInstruction(std::string op_name, chunk_iter* iter,
                                Chunk* chunk) {
    uint8_t constant = (*iter)[1];
    uint8_t argCount = (*iter)[2];
    std::cout << " " << op_name << " " << (int)argCount;
    std::cout << constant << " ";
    Value::printValue(chunk->getValue(constant));
    std::cout << std::endl;
    *iter += 3;
}

std::string get_op_code(uint8_t code){
       switch (code){
        case OP_RETURN: return "OP_RETURN";
        case OP_CONSTANT: return "OP_CONSTANT";
        case OP_NIL: return "OP_NIL";
        case OP_TRUE: return "OP_TRUE";
        case OP_FALSE: return "OP_FALSE";
        case OP_POP: return "OP_POP";
        case OP_GET_LOCAL: return "OP_GET_LOCAL";
        case OP_SET_LOCAL: return "OP_SET_LOCAL";
        case OP_GET_GLOBAL: return "OP_GET_GLOBAL";
        case OP_DEFINE_GLOBAL: return "OP_DEFINE_GLOBAL";
        case OP_SET_GLOBAL: return "OP_SET_GLOBAL";
        case OP_GET_PROPERTY: return "OP_GET_PROPERTY";
        case OP_SET_PROPERTY: return "OP_SET_PROPERTY";
        case OP_EQUAL: return "OP_EQUAL";
        case OP_GREATER: return "OP_GREATER";
        case OP_LESS: return "OP_LESS";
        case OP_ADD: return "OP_ADD";
        case OP_SUBTRACT: return "OP_SUBTRACT";
        case OP_MULTIPLY: return "OP_MULTIPLY";
        case OP_DIVIDE: return "OP_DIVIDE";
        case OP_NOT: return "OP_NOT";
        case OP_NEGATE: return "OP_NEGATE";
        case OP_PRINT: return "OP_PRINT";
        case OP_JUMP: return "OP_JUMP";
        case OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OP_LOOP: return "OP_LOOP";
        case OP_CALL: return "OP_CALL";
        case OP_INVOKE: return "OP_INVOKE";
        case OP_CLASS: return "OP_CLASS";
        case OP_CLOSURE: return "OP_CLOSURE";
        case OP_METHOD: return "OP_METHOD";
        case OP_INHERIT: return "OP_INHERIT";
        case OP_GET_SUPER: return "OP_GET_SUPER";
        case OP_SUPER_INVOKE: return "OP_SUPER_INVOKE";
        case OP_GET_UPVALUE: return "OP_GET_UPVALUE";
        case OP_SET_UPVALUE: return "OP_SET_UPVALUE";
        default: return "unknown operation code";
    }
}