#include <iostream>
#include "chunk.hpp"
#include "debug.hpp"
#include "vm.hpp"

int main(int, char**) {
    Chunk chunk;
    VirtualMachine vm;
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeValue(1.2, 123);

    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeValue(3.4, 123);

    chunk.writeChunk(OP_ADD, 123);

    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeValue(5.6, 123);

    chunk.writeChunk(OP_DIVIDE, 123);

    chunk.writeChunk(OP_NEGATE, 123);

    chunk.writeChunk(OP_RETURN, 123);
    vm.interpret(&chunk);
}

// a = 1.2
// b = 3.4
// c = a+b
// d = 5.6
// return - (b / d)
