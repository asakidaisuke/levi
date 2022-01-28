#include <iostream>
#include <functional>
#include "compiler.hpp"
#include "scanner.hpp"


using namespace std::placeholders;

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

bool Compiler::match(TokenType type){
    if(!check(type)) return false;
    advance();
    return true;
}

bool Compiler::check(TokenType type){
    return parser.current.type == type;
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
        case TOKEN_BANG_EQUAL:{
            emitByte(OP_EQUAL);
            emitByte(OP_NOT);
            break;}
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL:{
            emitByte(OP_LESS);
            emitByte(OP_NOT);
            break;}
        case TOKEN_LESS:
            emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:{
            emitByte(OP_GREATER);
            emitByte(OP_NOT);
            break;}
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

void Compiler::printStatement(){
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

void Compiler::expressionStatement(){
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Compiler::statement(){
    if(match(TOKEN_PRINT)){
        printStatement();
    }else{
        expressionStatement();
    }
}

void Compiler::varDeclaration(){
    uint8_t global = parseVariable("Expect variable name.");
    if(match(TOKEN_EQUAL)){
        expression();
    }else{
        emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    defineVariable(global);
}

void Compiler::declaration(){
    if (match(TOKEN_VAR)){
        varDeclaration();
    }else{
        statement();
    }
    if(parser.panicMode) synchronize();
}

void Compiler::synchronize(){
    parser.panicMode = false;

    while(parser.current.type != TOKEN_EOF){
        if(parser.previous.type == TOKEN_SEMICOLON) return;
        switch(parser.current.type){
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
      default:
        ;
        }
        advance();
    }
}

uint8_t Compiler::makeConstant(value_t val) {
  int constant = chunk->addConstantToValue(val);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constant;
}

void Compiler::string(){
    ObjString* objString = new ObjString;
    Object::getObjString(
        std::string(parser.previous.start + 1,
                        parser.previous.start + parser.previous.length-1),
        objString
    );
    emitConstant(OBJ_VAL(objString));
}

void Compiler::namedVariable(Token name, bool canAssign){
    uint8_t arg = identifierConstant(&name);
    if(canAssign && match(TOKEN_EQUAL)){
        expression();
        emitByte(OP_GET_GLOBAL);
        emitByte(arg);
    }else{
        emitByte(OP_GET_GLOBAL);
        emitByte(arg);
    }
}

void Compiler::variable(bool canAssign){
    namedVariable(parser.previous, canAssign);
}

void Compiler::number(){
    double value = std::stod(
        std::string(
            parser.previous.start, 
            parser.previous.start + parser.previous.length
        ));
    emitConstant(NUMBER_VAL(value));
}

void Compiler::literal(){
    switch(parser.previous.type){
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NIL: emitByte(OP_NIL); break;
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        default: return;
    }
}

void Compiler::emitConstant(value_t input_val){
    chunk->writeChunk(OP_CONSTANT, parser.previous.line);
    chunk->writeValue(input_val, parser.previous.line);
}

void Compiler::emitByte(uint8_t op_code){
    chunk->writeChunk(op_code, parser.previous.line);
}

void Compiler::emitReturn(){
    emitByte(OP_RETURN);
}


void Compiler::grouping(){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary(){
    TokenType operatorType = parser.previous.type;
    expression();
    switch(operatorType){
        case TOKEN_BANG:{
            emitByte(OP_NOT);
            break;
        }
        case TOKEN_MINUS: {
            emitByte(OP_NEGATE);
            break;}
        default:
            return;
    }
}

uint8_t Compiler::parseVariable(std::string errorMessage){
    consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&parser.previous);
}

void Compiler::defineVariable(uint8_t global){
    emitByte(OP_DEFINE_GLOBAL);
    emitByte(global);
}

uint8_t Compiler::identifierConstant(Token* name){
    ObjString* objString = new ObjString;
    Object::getObjString(
        std::string(name->start, name->start + name->length),
        objString
    );
    return makeConstant(OBJ_VAL(objString));
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
    // expression();
    // consume(TOKEN_EOF, "Expect end of expression.");
    while(!match(TOKEN_EOF)){
        declaration();
    }
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
        std::bind(&Compiler::unary, this), NULL, PREC_NONE};
    rules[TOKEN_BANG_EQUAL] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_EQUALITY};
    rules[TOKEN_EQUAL] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_EQUAL_EQUAL] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_COMPARISON};
    rules[TOKEN_GREATER] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_COMPARISON};
    rules[TOKEN_GREATER_EQUAL] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_COMPARISON};
    rules[TOKEN_LESS] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_COMPARISON};
    rules[TOKEN_LESS_EQUAL] = ParseRule{
        NULL, std::bind(&Compiler::binary, this), PREC_COMPARISON};
    rules[TOKEN_IDENTIFIER] = ParseRule{
        std::bind(&Compiler::variable, this, true), NULL, PREC_NONE};
    rules[TOKEN_STRING] = ParseRule{
        std::bind(&Compiler::string, this), NULL, PREC_NONE};
    rules[TOKEN_NUMBER] = ParseRule{
        std::bind(&Compiler::number, this), NULL, PREC_NONE};
    rules[TOKEN_AND] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_CLASS] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_ELSE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_FALSE] = ParseRule{
        std::bind(&Compiler::literal, this), NULL, PREC_NONE};
    rules[TOKEN_FOR] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_FUN] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_IF] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_NIL] = ParseRule{
        std::bind(&Compiler::literal, this), NULL, PREC_NONE};
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
        std::bind(&Compiler::literal, this), NULL, PREC_NONE};
    rules[TOKEN_VAR] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_WHILE] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_ERROR] = ParseRule{
        NULL, NULL, PREC_NONE};
    rules[TOKEN_EOF] = ParseRule{
        NULL, NULL, PREC_NONE};
}
