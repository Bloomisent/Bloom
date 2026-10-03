#include "include/lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- helpers ---------- */

static char *dup_range(const char *s, size_t start, size_t end)
{
    size_t n = end - start;
    char *p = (char *)malloc(n + 1);
    if (!p) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    memcpy(p, s + start, n);
    p[n] = '\0';
    return p;
}

static char peek(const Lexer *l)      { return l->source[l->pos]; }
static char peek_next(const Lexer *l) { return l->source[l->pos + 1]; }

/* Consume one character, tracking line/column. */
static void advance(Lexer *l)
{
    char c = l->source[l->pos++];
    if (c == '\n') {
        l->line++;
        l->column = 1;
    } else {
        l->column++;
    }
}

/* Consume the next character if it equals `expected`. */
static int match(Lexer *l, char expected)
{
    if (peek(l) != expected) return 0;
    advance(l);
    return 1;
}

static void skip_space_and_comments(Lexer *l){
    for (;;) {
        while (isspace((unsigned char)peek(l))){
            advance(l);
        };

        if (peek(l) == '/' && peek_next(l) == '/') {
            while (peek(l) && peek(l) != '\n')
                advance(l);
            continue;
        }
        break;
    }
}

static Token make_token(TokenType type, int line, int column){
    Token t = {0};
    t.type   = type;
    t.line   = line;
    t.column = column;
    return t;
}

static TokenType keyword_type(const char *text){
    if (!strcmp(text, "let"))   return TOK_LET;
    if (!strcmp(text, "print")) return TOK_PRINT;
    if (!strcmp(text, "if"))    return TOK_IF;
    if (!strcmp(text, "else"))  return TOK_ELSE;
    if (!strcmp(text, "while")) return TOK_WHILE;
    return TOK_IDENT;
}

/* ---------- public API ---------- */

void lexer_init(Lexer *lexer, const char *source){
    lexer->source = source;
    lexer->pos    = 0;
    lexer->line   = 1;
    lexer->column = 1;
}

Token lexer_next(Lexer *l){
    skip_space_and_comments(l);

    int  line   = l->line;
    int  column = l->column;
    char c      = peek(l);

    if (!c)
        return make_token(TOK_EOF, line, column);

    /* integer literal */
    if (isdigit((unsigned char)c)) {
        size_t start = l->pos;
        while (isdigit((unsigned char)peek(l))) {
            advance(l);
        };
        Token t = make_token(TOK_INT, line, column);
        t.text  = dup_range(l->source, start, l->pos);
        t.value = strtoll(t.text, NULL, 10);
        return t;
    };

    /* string literal */
    if (c == '"') {
        advance(l);
        size_t start = l->pos;
        while (peek(l) && peek(l) != '"') {
            advance(l);
        };
        advance(l);
        Token t = make_token(TOK_STR, line, column);
        t.text  = dup_range(l->source, start, l->pos-2);
        return t;
    };

    /* identifier or keyword */
    if (isalpha((unsigned char)c) || c == '_') {
        size_t start = l->pos;
        while (isalnum((unsigned char)peek(l)) || peek(l) == '_')
            advance(l);

        Token t = make_token(TOK_IDENT, line, column);
        t.text  = dup_range(l->source, start, l->pos);
        t.type  = keyword_type(t.text);
        return t;
    }

    /* operators and punctuation */
    advance(l);
    Token t = make_token(TOK_EOF, line, column);

    switch (c) {
    case '+': t.type = TOK_PLUS;    break;
    case '-': t.type = TOK_MINUS;   break;
    case '*': t.type = TOK_STAR;    break;
    case '/': t.type = TOK_SLASH;   break;
    case '%': t.type = TOK_PERCENT; break;
    case '(': t.type = TOK_LPAREN;  break;
    case ')': t.type = TOK_RPAREN;  break;
    case '{': t.type = TOK_LBRACE;  break;
    case '}': t.type = TOK_RBRACE;  break;
    case ';': t.type = TOK_SEMI;    break;

    case '!': t.type = match(l, '=') ? TOK_NE     : TOK_BANG;   break;
    case '=': t.type = match(l, '=') ? TOK_EQ     : TOK_ASSIGN; break;
    case '<': t.type = match(l, '=') ? TOK_LE     : TOK_LT;     break;
    case '>': t.type = match(l, '=') ? TOK_GE     : TOK_GT;     break;

    default:
        fprintf(stderr, "Lexer error at %d:%d: unexpected character '%c'\n",
                line, column, c);
        exit(1);
    }
    return t;
}

void token_free(Token *token) {
    free(token->text);
    token->text = NULL;
}

const char *token_type_name(TokenType type){
    switch (type) {
        case TOK_EOF:     return "end of file";
        case TOK_INT:     return "integer";
        case TOK_STR:     return "string";
        case TOK_IDENT:   return "identifier";
        case TOK_LET:     return "let";
        case TOK_PRINT:   return "print";
        case TOK_IF:      return "if";
        case TOK_ELSE:    return "else";
        case TOK_WHILE:   return "while";
        case TOK_PLUS:    return "+";
        case TOK_MINUS:   return "-";
        case TOK_STAR:    return "*";
        case TOK_SLASH:   return "/";
        case TOK_PERCENT: return "%";
        case TOK_ASSIGN:  return "=";
        case TOK_EQ:      return "==";
        case TOK_NE:      return "!=";
        case TOK_LT:      return "<";
        case TOK_LE:      return "<=";
        case TOK_GT:      return ">";
        case TOK_GE:      return ">=";
        case TOK_BANG:    return "!";
        case TOK_LPAREN:  return "(";
        case TOK_RPAREN:  return ")";
        case TOK_LBRACE:  return "{";
        case TOK_RBRACE:  return "}";
        case TOK_SEMI:    return ";";
    }
    return "?";
}
