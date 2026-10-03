#ifndef TINYC_PARSER_H
#define TINYC_PARSER_H
#include "ast.h"
#include "lexer.h"

typedef struct { Lexer lexer; Token current; } Parser;
void parser_init(Parser *p, const char *source);
Stmt *parser_parse_program(Parser *p);
void parser_destroy(Parser *p);

#endif
