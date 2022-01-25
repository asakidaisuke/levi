#ifndef LEVI_COMPILER_H
#define LEVI_COMPILER_H

#include <string>
#include <unordered_map>
#include <functional>
#include "chunk.hpp"
#include "scanner.hpp"
#include "common.hpp"
#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif


enum Precedence{
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
};

struct Parser{
    Token current;
    Token previous;
    bool hadError{false};
    bool panicMode{false};
};

struct ParseRule{
    std::function<void()> prefix;
    std::function<void()> infix;
    Precedence precedence;
};

class Compiler{
    public:
        bool compile(std::string);
        inline std::unique_ptr<Chunk> get_chunk(){
            return std::move(chunk);
        }
        Compiler(std::string source) : scanner(&source){
            chunk = std::make_unique<Chunk>();
            init_rules();
            }
    private:
        void advance();
        inline void errorAtCurrent(std::string message){
            errorAt(&parser.current, message);
        }
        inline void error(std::string message){
            errorAt(&parser.current, message);
        }
        void errorAt(Token* token, std::string message);
        void consume(TokenType type, std::string message);
        void emitByte(uint8_t byte);
        void expression();
        void number();
        void literal();
        void endCompiler();
        void emitReturn();
        void emitByte();
        // void emitBytes(uint8_t byte1, uint8_t byte2);
        void emitConstant(value_t input_val);
        void grouping();
        void unary();
        void binary();
        void parsePrecedence(Precedence precedence);
        void init_rules();
        void makeConstant(value_t input_val);
        ParseRule* getRule(TokenType type);
        Chunk* currentChunk();
        std::unique_ptr<Chunk> chunk;
        Parser parser;
        Scanner scanner;
        std::unordered_map<TokenType, ParseRule> rules;
};

#endif