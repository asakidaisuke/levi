#include <iostream>
#include <functional>
#include "compiler.hpp"
#include "scanner.hpp"


using namespace std::placeholders;


void Compiler::setCurrent(Compiler* compiler){
    currentCompiler = compiler;
}

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
    return currentCompiler->compilerState.function->chunk.get();
}

ObjFunction* Compiler::endCompiler(){
    emitReturn();
    ObjFunction* local_function = currentCompiler->compilerState.function;
    #ifdef DEBUG_PRINT_CODE
        if (!parser.hadError){
            disassembleChunk("code", local_function->chunk.get());
        }
    #endif
    return local_function;
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
        case TOKEN_PLUS:    emitByte(OP_ADD); break;
        case TOKEN_MINUS:   emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:    emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:   emitByte(OP_DIVIDE); break;
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

void Compiler::forStatement(){
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if(match(TOKEN_SEMICOLON)){

    } else if(match(TOKEN_VAR)){
        varDeclaration();
    } else {
        expressionStatement();
    }
    int loopStart = currentChunk()->getChunk()->size();
    int exitJump = -1;
    if(!match(TOKEN_SEMICOLON)){
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }

    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->getChunk()->size();
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if(exitJump != -1){
        patchJump(exitJump);
        emitByte(OP_POP);
    }
    endScope();
}

void Compiler::whileStatement(){
    int loopStart = currentChunk()->getChunk()->size();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

void Compiler::expressionStatement(){
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Compiler::ifStatement(){
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    int elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);
    emitByte(OP_POP);

    if(match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}


void Compiler::returnStatement(){
    if(currentCompiler->compilerState.type == TYPE_SCRIPT){
        error("Can't return from top-level code.");
    }

    if(match(TOKEN_RETURN)){
        emitReturn();
    }else{
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }
}

void Compiler::statement(){
    if(match(TOKEN_PRINT)){
        printStatement();
    }else if(match(TOKEN_IF)){
        ifStatement();
    }else if(match(TOKEN_WHILE)){
        whileStatement();
    }else if(match(TOKEN_FOR)){
        forStatement();
    }else if(match(TOKEN_RETURN)){
        returnStatement();
    }else if(match(TOKEN_LEFT_BRACE)){
        beginScope();
        block();
        endScope();
    }else{
        expressionStatement();
    }
}

void Compiler::beginScope(){
    currentCompiler->compilerState.scopeDepth++;
}

void Compiler::endScope(){
    currentCompiler->compilerState.scopeDepth--;

    while(compilerState.localCount > 0 && compilerState.locals[compilerState.localCount-1].depth >
                compilerState.scopeDepth){
            if (currentCompiler->compilerState.locals[currentCompiler->compilerState.localCount-1].isCaptured){
                emitByte(OP_CLOSE_UPVALUE);
            }else{
                emitByte(OP_POP);
            }
            compilerState.localCount--;
        }
}

void Compiler::block(){
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
        declaration();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block." );
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

void Compiler::funDeclaration(){
    uint8_t global = parseVariable("Expect function name.");
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

void Compiler::declaration(){
    if (match(TOKEN_VAR)){
        varDeclaration();
    }else if(match(TOKEN_FUN)){
        funDeclaration();
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
  int constant = currentChunk()->addConstantToValue(val);
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

int Compiler::resolveLocal(CompilerState* compilerState, Token* name){
    for(int i = compilerState->localCount-1; i >=0; i--){
        Local* local = &compilerState->locals[i];
        if (identifierEqual(name, &local->name)){
            if(local->depth == -1){
                error("Cant't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

int Compiler::resolveUpvalue(Compiler* compiler, Token* name){
    if(compiler->encloseCompiler == NULL) return -1;

    int local = resolveLocal(&compiler->encloseCompiler->compilerState, name);
    if(local != -1){
        compiler->encloseCompiler->compilerState.locals[local].isCaptured = true;
        return addUpvalue(compiler, (uint8_t)local, true);
    }

    return -1;
}

int Compiler::addUpvalue(Compiler* compiler, uint8_t index, bool isLocal){
    int upvalueCount = compiler->compilerState.function->upvalueCount;

    for (int i = 0; i < upvalueCount; i++) {
        Upvalue* upvalue = &compiler->compilerState.upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
        return i;
        }
    }

    if(upvalueCount == UINT8_COUNT){
        error("Too many closure variables in function.");
        return 0;
    }

    compiler->compilerState.upvalues[upvalueCount].isLocal = isLocal;
    compiler->compilerState.upvalues[upvalueCount].index = index;
    return compiler->compilerState.function->upvalueCount++;
}

void Compiler::namedVariable(Token name, bool canAssign){
    uint8_t getOp, setOp;
    int arg = resolveLocal(&currentCompiler->compilerState, &name);
    if(arg != -1){
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }else if((arg = resolveUpvalue(currentCompiler, &name)) != -1){
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    }else{
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    if(canAssign && match(TOKEN_EQUAL)){
        expression();
        emitByte(setOp);
        emitByte((uint8_t)arg);
    }else{
        emitByte(getOp);
        emitByte((uint8_t)arg);
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

void Compiler::function(FunctionType type){
    Compiler compiler(source);
    Compiler* temp = currentCompiler; // move current one
    Compiler** temp_ptr = &currentCompiler;
    Compiler* new_ptr = &compiler;
    *temp_ptr = new_ptr;
    currentCompiler->encloseCompiler = temp;
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if(!check(TOKEN_RIGHT_PAREN)){
        do {
            currentCompiler->compilerState.function->arity++;
            if(currentCompiler->compilerState.function->arity > 255){
                errorAtCurrent("Can't have more than parameter name.");
            }
            uint8_t constant = parseVariable("Expect parameter name.");
            defineVariable(constant);
        } while(match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    ObjFunction* function = endCompiler();
    currentCompiler = currentCompiler->encloseCompiler; // regain current one
    emitByte(OP_CLOSURE);
    emitByte(makeConstant(OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++){
        emitByte(compiler.compilerState.upvalues[i].isLocal ? 1 : 0);
        emitByte(compiler.compilerState.upvalues[i].index);
    }
}

void Compiler::call(bool canAssign){
    uint8_t argCount = argumentList();
    emitByte(OP_CALL);
    emitByte(argCount);
}

uint8_t Compiler::argumentList(){
    uint8_t argCount = 0;
    if(!check(TOKEN_RIGHT_PAREN)){
        do {
            expression();
            if(argCount == 255){
                error("Can't have more than 255 arguments.");
            }
            argCount++;
        } while(match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

void Compiler::patchJump(int offset){
    int jump = currentChunk()->getChunk()->size() - offset - 2;
    if(jump > UINT16_MAX){
        error("Too much code to jump over.");
    }
    (*currentChunk()->getChunk())[offset] = (jump >> 8) & 0xff;
    (*currentChunk()->getChunk())[offset+1] = jump & 0xff;
}

int Compiler::emitJump(uint8_t instruction){
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->getChunk()->size() - 2;
}

void Compiler::emitConstant(value_t input_val){
    currentChunk()->writeChunk(OP_CONSTANT, parser.previous.line);
    currentChunk()->writeValue(input_val, parser.previous.line);
}

void Compiler::emitByte(uint8_t op_code){
    currentChunk()->writeChunk(op_code, parser.previous.line);
}

void Compiler::emitLoop(int loopStart){
    emitByte(OP_LOOP);

    int offset = currentChunk()->getChunk()->size() - loopStart + 2;
    if(offset > UINT16_MAX) error("Loop body too large.");
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
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
    declareVariable();
    if(currentCompiler->compilerState.scopeDepth > 0) return 0;
    return identifierConstant(&parser.previous);
}

void Compiler::markInitialized(){
    currentCompiler->compilerState.locals[currentCompiler->compilerState.localCount-1].depth = currentCompiler->compilerState.scopeDepth;
}

bool Compiler::identifierEqual(Token* a, Token* b){
    if(a->length != b->length) return false;
    return std::string(a->start, a->start + a->length) == \
            std::string(b->start, b->start + b->length);
}

void Compiler::declareVariable(){
    if(currentCompiler->compilerState.scopeDepth == 0) return;

    Token* name = &parser.previous;
    // look above level local variable
    for(int i=currentCompiler->compilerState.localCount-1; i>=0; i--){
        Local* local = &currentCompiler->compilerState.locals[i];
        if(local->depth != -1 && local->depth < currentCompiler->compilerState.scopeDepth){
            break;
        }

        if(identifierEqual(name, &local->name)){
            error("Already a variable with this name in this scope.");
        }
    }
    addLocal(*name);
}

void Compiler::addLocal(Token name){
    if(currentCompiler->compilerState.localCount == UINT8_COUNT){
        error("Too many local variables in function.");
        return;
    }
    Local* local = &currentCompiler->compilerState.locals[currentCompiler->compilerState.localCount++];
    local->name = name;
    local->depth = currentCompiler->compilerState.scopeDepth;
    local->isCaptured = false;
}

void Compiler::defineVariable(uint8_t global){
    if(currentCompiler->compilerState.scopeDepth > 0){
        markInitialized();
        return;
    }
    emitByte(OP_DEFINE_GLOBAL);
    emitByte(global);
}

void Compiler::and_(bool canAssign){
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

void Compiler::or_(bool canAssign){
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
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

ObjFunction* Compiler::compile(std::string source){
    advance();
    while(!match(TOKEN_EOF)){
        declaration();
    }
    ObjFunction* function = endCompiler();
    if(parser.hadError) return NULL;
    return function;
}

void Compiler::init_rules(){
    rules[TOKEN_LEFT_PAREN] = ParseRule{
        std::bind(&Compiler::grouping, this), std::bind(&Compiler::call, this, true), PREC_CALL};
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
        NULL, std::bind(&Compiler::and_, this, true), PREC_AND};
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
        NULL, std::bind(&Compiler::or_, this, true), PREC_OR};
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
