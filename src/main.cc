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

int main(int argc, const char* argv[]){
    Chunk chunk;
    VirtualMachine vm;
    argc = 2;
    argv[1] = "../class.lev";
    if (argc == 1){
        repl();
    }else if (argc == 2){
        runFile(argv[1]);
    }else{
        std::cout << "Usage: clox [path] \n" << std::endl;
    }
}
