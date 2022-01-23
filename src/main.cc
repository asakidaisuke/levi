#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "chunk.hpp"
#include "debug.hpp"
#include "vm.hpp"

#define MAX_LINE_LEN 100


std::string line;

std::string readFile(std::string path){
    std::ifstream ifile;
    std::string str;
    std::string buffer;
    ifile.open(path);
    while(std::getline(ifile, str)){
        buffer += str;
        buffer += "\n";
    }
    return buffer;
}

void runFile(std::string path){
    std::string source = readFile(path);
    VirtualMachine vm;
    InterpretResult result = vm.interpret(source);
    // if (result == INTERPRET_COMPILE_ERROR) exit(65);
    // if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

static void repl(){
    std::string line;
    for (;;) {
        char line[MAX_LINE_LEN];
        std::cout << "> ";
        std::cin.getline (line, MAX_LINE_LEN);
        std::cout << line << std::endl;
    }
}

int main(int argc, char* argv[]){
    Chunk chunk;
    VirtualMachine vm;
    // chunk.writeChunk(OP_CONSTANT, 123);
    // chunk.writeValue(1.2, 123);

    // chunk.writeChunk(OP_CONSTANT, 123);
    // chunk.writeValue(3.4, 123);

    // chunk.writeChunk(OP_ADD, 123);

    // chunk.writeChunk(OP_CONSTANT, 123);
    // chunk.writeValue(5.6, 123);

    // chunk.writeChunk(OP_DIVIDE, 123);

    // chunk.writeChunk(OP_NEGATE, 123);

    // chunk.writeChunk(OP_RETURN, 123);
    // vm.interpret(&chunk);
    // disassembleChunk("test chunk", &chunk);
    argc = 2;
    argv[1] = "../main.lev";
    if (argc == 1){
        repl();
    }else if (argc == 2){
        runFile(argv[1]);
    }else{
        std::cout << "Usage: clox [path] \n" << std::endl;
    }
}

// a = 1.2
// b = 3.4
// c = a+b
// d = 5.6
// return - (b / d)
