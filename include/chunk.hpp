#ifndef LEVI_CHUNK_H
#define LEVI_CHUNK_H

#include <vector>
#include <memory>

using chunk_t = std::vector<uint8_t>;

typedef enum {
    OP_RETURN,s
} OpCode;


class Chunk{
    public:
        void writeChunk(uint8_t byte);
        const chunk_t getChunk();
    private:
        chunk_t chunk_stack;
};


#endif