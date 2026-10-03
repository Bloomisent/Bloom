#ifndef TINYC_LEXER_H
#define TINYC_LEXER_H
#include "token.h"
#include <stddef.h>

typedef struct {
    const char *source;
    size_t pos;
    int line;
    int column;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_next(Lexer *lexer);
void token_free(Token *token);

#endif
