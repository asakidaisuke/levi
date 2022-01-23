#include <iostream>
#include <functional>
#include "compiler.hpp"
#include "scanner.hpp"


// void Compiler::compile(std::string source){
//     int line = -1;
//     Scanner scanner(&source);
//     for(;;){
//         Token token = scanner.scanToken();
//         if (token.line != line){
//             std::cout << token.line;
//             line = token.line;
//         }else{
//             std::cout << "|";
//         }
//         std::cout << " " << token.type << " ";
//         // ここ
//         for (int i = 0; i < token.length; i++)
//             std::cout << token.start[i];
//         // std::cout << *token.start.begin();
//         std::cout<< std::endl;
//         if(token.type == TOKEN_EOF) break;
//     }
// }


// ここハッシュマップ使うとか、そのあとコンパイル通す、でデバッグ
// void Compiler::init_rules(){
//     rules = {
//         [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
//         [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, // [big]
//         [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
//         [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
//         [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
//         [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
//         [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
//         [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
//         [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
//     };
// }

void Compiler::advance(){
    parser.previous = parser.current;
    for(;;){
        parser.current = scanner.scanToken();
        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(
            std::string(
                parser.current.start,
                parser.current.start + parser.current.length)
            );
    }
}

void Compiler::consume(TokenType type, std::string message){
    if(parser.current.type == type){
        advance();
        return;
    }
    errorAtCurrent(message);
}

Chunk* Compiler::currentChunk(){
    return chunk.get();
}

void Compiler::endCompiler(){
    emitReturn();
    #ifdef DEBUG_PRINT_CODE
        if (!parser.hadError){
            disassembleChunk("code", currentChunk());
        }
    #endif
}

void Compiler::binary(){
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));
    switch(operatorType){
        // case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        // case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
        // case TOKEN_GREATER:       emitByte(OP_GREATER); break;
        // case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        // case TOKEN_LESS:          emitByte(OP_LESS); break;
        // case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        default: return; 
    }
}

void Compiler::expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::number(){
    double value = std::stod(
        std::string(
            parser.previous.start, 
            parser.previous.start + parser.previous.length
        ));
    emitConstant(value);
}

void Compiler::emitConstant(value_t input_val){
    // emitBytes(OP_CONSTANT, makeConstant(value));
    chunk->writeChunk(OP_CONSTANT, parser.previous.line);
    chunk->writeValue(input_val, parser.previous.line);
}

// void Compiler::emitBytes(uint8_t byte1, uint8_t byte2){
//     emitByte(byte1);
//     emitByte(byte2);
// }


// void Compiler::emitBytes(uint8_t byte1, uint8_t byte2){
//     emitByte(byte1);
//     emitByte(byte2);
// }

void Compiler::emitByte(uint8_t op_code){
    chunk->writeChunk(op_code, parser.previous.line);
}

void Compiler::emitReturn(){
    emitByte(OP_RETURN);
}

// void Compiler::makeConstant(value_t input_val){
//     chunk->writeValue(input_val, parser.previous.line);
//     if(chunk->getValueSize()> UINT8_MAX){
//         error("Too many constant in one chunk.");
//         return 0;
//     }
// }

void Compiler::grouping(){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary(){
    TokenType operatorType = parser.previous.type;
    expression();
    switch(operatorType){
        case TOKEN_MINUS: 
            emitByte(OP_NEGATE);
            break;
        default:
            return;
    }
}

void Compiler::parsePrecedence(Precedence precedence){
    advance();
    auto prefixRule = getRule(parser.previous.type)->prefix;
    if(prefixRule == NULL){
        error("Expect expression.");
        return;
    }
    prefixRule();

    while(precedence <= getRule(parser.current.type)->precedence){
        advance();
        auto infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

ParseRule* Compiler::getRule(TokenType type){
    return &rules[type];
}

void Compiler::errorAt(Token* token, std::string message){
    if(parser.panicMode) return;
    parser.panicMode = true;

    std::cout << "[line " << token->line << "] Error";
    if(token->type == TOKEN_EOF){
        std::cout << " at end";
    }else if(token->type == TOKEN_ERROR){
        // Nothing
    }else{
        std::cout << ": " << message << std::endl;
    }
    parser.hadError = true;
}

bool Compiler::compile(std::string source){
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}

void Compiler::init_rules(){
    rules[TOKEN_LEFT_PAREN] = ParseRule{
        std::bind(&Compiler::grouping, this), NULL, PREC_NONE};
    rules[TOKEN_RIGHT_PAREN] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_LEFT_BRACE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_RIGHT_BRACE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_COMMA] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_DOT] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_MINUS] = ParseRule{
        std::bind(&Compiler::unary, this), std::bind(&Compiler::binary, this), PREC_TERM};
    rules[TOKEN_PLUS] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_TERM};
    rules[TOKEN_SEMICOLON] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_SLASH] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_FACTOR};
    rules[TOKEN_STAR] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_FACTOR};
    rules[TOKEN_BANG] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_BANG_EQUAL] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_EQUAL] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_EQUAL_EQUAL] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_GREATER] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_GREATER_EQUAL] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_LESS] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_LESS_EQUAL] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_IDENTIFIER] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_STRING] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_NUMBER] = ParseRule{
        std::bind(&Compiler::number, this), NULL, PREC_NONE};
    rules[TOKEN_AND] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_CLASS] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_ELSE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_FALSE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_FOR] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_FUN] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_IF] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_NIL] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_OR] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_PRINT] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_RETURN] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_SUPER] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_THIS] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_TRUE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_VAR] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_WHILE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_ERROR] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_EOF] = ParseRule{
        NULL, NULL, PREC_NONE};
}
