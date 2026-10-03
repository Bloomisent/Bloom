#ifndef TINYC_TOKEN_H
#define TINYC_TOKEN_H

typedef enum {
    TOK_EOF,
    TOK_INT,
    TOK_STR,
    TOK_IDENT,
    TOK_LET,
    TOK_PRINT,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_ASSIGN,
    TOK_EQ,
    TOK_NE,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,
    TOK_BANG,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMI
} TokenType;

typedef struct {
    TokenType type;
    long long value;
    char *text;
    int line;
    int column;
} Token;

const char *token_type_name(TokenType type);

#endif
