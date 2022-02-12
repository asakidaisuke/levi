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
#define UINT8_COUNT (UINT8_MAX + 1)
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

enum FunctionType{
    TYPE_FUNCTION,
    TYPE_SCRIPT
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
    int depth{0};
    bool isCaptured{false};
};

class Compiler;

struct Upvalue{
    uint8_t index;
    bool isLocal;
};

struct CompilerState{
    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount{0};
    int scopeDepth{0};
    Upvalue upvalues[UINT8_COUNT];
};

class Compiler{
    public:
        ObjFunction* compile(std::string);
        void setCurrent(Compiler* compiler);
        Compiler(std::string source) : scanner(&source){
            init_rules();

            compilerState.function = new ObjFunction;
            compilerState.function->chunk = std::make_unique<Chunk>();
            source = source;
            currentCompiler = this;
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
        void function(FunctionType);
        void call(bool);
        uint8_t argumentList();
        void printStatement();
        void whileStatement();
        void ifStatement();
        void forStatement();
        void expressionStatement();
        void returnStatement();
        void statement();
        void varDeclaration();
        void funDeclaration();
        void declaration();
        void synchronize();
        void defineVariable(uint8_t);
        void and_(bool);
        void or_(bool);
        uint8_t identifierConstant(Token*);
        void namedVariable(Token, bool);
        ObjFunction* endCompiler();
        void emitReturn();
        void emitByte();
        void emitLoop(int);
        void emitConstant(value_t input_val);
        int emitJump(uint8_t);
        void patchJump(int);
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
        int resolveUpvalue(Compiler*, Token*);
        int addUpvalue(Compiler*, uint8_t, bool);
        Chunk* currentChunk();
        // std::unique_ptr<Chunk> chunk;
        Parser parser;
        Scanner scanner;
        std::unordered_map<TokenType, ParseRule> rules;
        CompilerState compilerState;
        Compiler* currentCompiler;
        Compiler* encloseCompiler;
        std::string source;
};

#endif