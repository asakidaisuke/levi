#include <stdio.h>
#include <string.h>
#include "scanner.hpp"


Token Scanner::scanToken(){
    skipWhitespace();
    start = current;
    if(isAtEnd()) return makeToken(TOKEN_EOF);
    char c = advance();
    if(isAlpha(c)) return identifier();
    if(isDigit(c)) return numbers();
    switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return strings();
    };
    return errorToken("Unexpected charactor.");
}


bool Scanner::match(char expected){
    if(isAtEnd()) return false;
    if(*current != expected) return false;
    current++;
    return true;
}

Token Scanner::makeToken(TokenType type){
    Token token;
    token.type = type;
    token.start = start;
    token.length = (int)(current - start);
    token.line = line;
    return token;
}

Token Scanner::errorToken(std::string message){
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message.begin();
    token.length = message.length();
    token.line = line;
    return token;
}

void Scanner::skipWhitespace(){
    for(;;){
        char c = peek();
        switch(c){
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/':
                if(peekNext() == '/'){
                    while(peek() != '\n' && !isAtEnd())
                        advance();
                }else{
                    return;
                }
                break;
            default:
                return; 
        }
    }
}

Token Scanner::strings(){
    while(peek() != '"' && !isAtEnd()){
        if(peek() == '\n') line++;
        advance();
    }

    if(isAtEnd()) return errorToken("Unterminated string.");

    advance();
    return makeToken(TOKEN_STRING);
}

Token Scanner::numbers(){
    while(isDigit(peek())) advance();
    if(peek() == '.' && isDigit(peekNext())){
        advance();
        while(isDigit(peek())) advance();
    }
    return makeToken(TOKEN_NUMBER);
}

Token Scanner::identifier(){
    while(isAlpha(peek()) || isDigit(peek())) 
        advance();
    return makeToken(identifierType());
}

TokenType Scanner::identifierType(){
    switch(start[0]){
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if(current - start > 1){
                switch(*(start+1)){
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if(current - start > 1){
                switch(*(start+1)){
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

TokenType Scanner::checkKeyword(
    int w_start, int w_length, std::string rest, TokenType type){
        std::string::iterator temp = start;
        if(current - start == w_start + w_length){
            temp++;
            for(int i=0; i<w_length; i++){
                if (*temp != rest[i]){
                    return TOKEN_IDENTIFIER;
                }
                temp++;
            }
            return type;
        }
        return TOKEN_IDENTIFIER;
    }