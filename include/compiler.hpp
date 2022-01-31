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
#include "object.hpp"
#define UINT8_COUT (UINT8_MAX + 1)
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

struct Local{
    Token name;
    int depth;
};

struct CompilerState{
    Local locals[UINT8_COUT];
    int localCount{0};
    int scopeDepth{0};
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
        bool match(TokenType);
        bool check(TokenType);
        void number();
        void string();
        void variable(bool);
        void literal();
        void printStatement();
        void expressionStatement();
        void statement();
        void varDeclaration();
        void declaration();
        void synchronize();
        void defineVariable(uint8_t);
        uint8_t identifierConstant(Token*);
        void namedVariable(Token, bool);
        void endCompiler();
        void emitReturn();
        void emitByte();
        void emitConstant(value_t input_val);
        void grouping();
        void unary();
        void binary();
        void block();
        void beginScope();
        void endScope();
        void declareVariable();
        void addLocal(Token);
        void identifierEqual();
        void markInitialized();
        uint8_t parseVariable(std::string);
        bool identifierEqual(Token*, Token*);
        void parsePrecedence(Precedence precedence);
        void init_rules();
        uint8_t makeConstant(value_t input_val);
        ParseRule* getRule(TokenType type);
        int resolveLocal(CompilerState* , Token* );
        Chunk* currentChunk();
        std::unique_ptr<Chunk> chunk;
        Parser parser;
        Scanner scanner;
        std::unordered_map<TokenType, ParseRule> rules;
        CompilerState compilerState;
};

#endif