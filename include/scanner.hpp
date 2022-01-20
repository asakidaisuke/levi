#ifndef LEVI_SCANNER_H
#define LEVI_SCANNER_H

#include <string>

enum TokenType{
    TOKEN_LEFT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_PAREN,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR, 
    TOKEN_FUN, 
    TOKEN_IF, 
    TOKEN_NIL, 
    TOKEN_OR,
    TOKEN_PRINT, 
    TOKEN_RETURN, 
    TOKEN_SUPER, 
    TOKEN_THIS,
    TOKEN_TRUE, 
    TOKEN_VAR, 
    TOKEN_WHILE,
    TOKEN_ERROR, 
    TOKEN_EOF
};

struct Token{
    TokenType type;
    std::string::iterator start;
    int length;
    int line;
};

class Scanner{
    public:
        Token scanToken();
        Scanner(std::string* source) : line(1) {
            current = source->begin();
        }
    
    private:
        void skipWhitespace();
        Token makeToken(TokenType message);
        Token errorToken(std::string message);
        Token strings();
        Token numbers();
        Token identifier();
        TokenType identifierType();
        TokenType checkKeyword(int start, int length, std::string rest, TokenType type);
        inline bool isAlpha(char c){
            return (c >= 'a' && c <= 'z') ||\
             (c >= 'A' && c <= 'Z') || c == '_';
        }
        inline bool isAtEnd(){
            return *current == '\0';
        }
        char advance(){
            current++;
            return current[-1];
        }
        bool match(char expected);
        inline char peek(){
            return *current;
        }
        inline char peekNext(){
            if(isAtEnd()) return '\0';
            return current[1];
        }
        inline bool isDigit(char c){
            return c >= '0' && c <= '9';
        }
        std::string::iterator start;
        std::string::iterator current;
        int line;
};

#endif