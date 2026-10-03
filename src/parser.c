#include "include/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Parser helpers                                                            */
/* ------------------------------------------------------------------------- */

static void next_token(Parser *parser){
    token_free(&parser->current);
    parser->current = lexer_next(&parser->lexer);
}

static void parser_error(Parser *parser, const char *message){
    fprintf(
        stderr,
        "Parser error at %d:%d: %s (got %s)\n",
        parser->current.line,
        parser->current.column,
        message,
        token_type_name(parser->current.type)
    );

    exit(1);
}

static void expect(Parser *parser, TokenType type){
    if (parser->current.type != type) {
        parser_error(parser, "unexpected token");
    }

    next_token(parser);
}

/* ------------------------------------------------------------------------- */
/* Expressions                                                               */
/* ------------------------------------------------------------------------- */

static Expr *expression(Parser *parser);

static Expr *parse_primary(Parser *parser){
    if (parser->current.type == TOK_INT) {
        long long value = parser->current.value;
        next_token(parser);
        return ast_int(value);
    }

    if (parser->current.type == TOK_STR) {
        char* str = strdup(parser->current.text);
        next_token(parser);
        Expr *expr = ast_str(str);
        free(str);
        return expr;
    }

    if (parser->current.type == TOK_IDENT) {
        char *name = strdup(parser->current.text);
        Expr *expr;
        next_token(parser);
        expr = ast_var(name);
        free(name);
        return expr;
    }

    if (parser->current.type == TOK_LPAREN) {
        Expr *expr;
        next_token(parser);
        expr = expression(parser);
        expect(parser, TOK_RPAREN);
        return expr;
    }

    parser_error(parser, "expected expression");

    return NULL;
}

static Expr *parse_unary(Parser *parser) {
    if (parser->current.type == TOK_MINUS) {
        next_token(parser);

        return ast_unary(UOP_NEG, parse_unary(parser));
    }

    if (parser->current.type == TOK_BANG) {
        next_token(parser);

        return ast_unary(UOP_NOT, parse_unary(parser));
    }

    return parse_primary(parser);
}

static Expr *parse_multiplication(Parser *parser){
    Expr *expr = parse_unary(parser);

    while (parser->current.type == TOK_STAR || parser->current.type == TOK_SLASH || parser->current.type == TOK_PERCENT) {
        TokenType token = parser->current.type;
        BinaryOp op;

        next_token(parser);

        if (token == TOK_STAR) {
            op = OP_MUL;
        } else if (token == TOK_SLASH) {
            op = OP_DIV;
        } else {
            op = OP_MOD;
        }

        expr = ast_binary(op, expr, parse_unary(parser));
    }

    return expr;
}

static Expr *parse_addition(Parser *parser){
    Expr *expr = parse_multiplication(parser);

    while (parser->current.type == TOK_PLUS || parser->current.type == TOK_MINUS) {
        TokenType token = parser->current.type;
        BinaryOp op;

        next_token(parser);

        if (token == TOK_PLUS) {
            op = OP_ADD;
        } else {
            op = OP_SUB;
        }

        expr = ast_binary(op, expr, parse_multiplication(parser));
    }

    return expr;
}

static Expr *parse_comparison(Parser *parser){
    Expr *expr = parse_addition(parser);

    while (parser->current.type == TOK_LT || parser->current.type == TOK_LE || parser->current.type == TOK_GT || parser->current.type == TOK_GE) {
        TokenType token = parser->current.type;
        BinaryOp op;

        next_token(parser);

        switch (token) {
            case TOK_LT:
                op = OP_LT;
                break;

            case TOK_LE:
                op = OP_LE;
                break;

            case TOK_GT:
                op = OP_GT;
                break;

            case TOK_GE:
                op = OP_GE;
                break;

            default:
                op = OP_LT;
                break;
        }

        expr = ast_binary(op, expr, parse_addition(parser));
    }

    return expr;
}

static Expr *parse_equality(Parser *parser){
    Expr *expr = parse_comparison(parser);

    while (parser->current.type == TOK_EQ || parser->current.type == TOK_NE) {
        TokenType token = parser->current.type;
        BinaryOp op;

        next_token(parser);

        if (token == TOK_EQ) {
            op = OP_EQ;
        } else {
            op = OP_NE;
        }

        expr = ast_binary(op, expr, parse_comparison(parser));
    }

    return expr;
}

static Expr *expression(Parser *parser){
    return parse_equality(parser);
}

/* ------------------------------------------------------------------------- */
/* Statements                                                                */
/* ------------------------------------------------------------------------- */

static Stmt *statement(Parser *parser);

static Stmt *parse_block(Parser *parser){
    Stmt *head = NULL;
    Stmt *tail = NULL;

    expect(parser, TOK_LBRACE);

    while (parser->current.type != TOK_RBRACE && parser->current.type != TOK_EOF) {
        Stmt *stmt = statement(parser);

        if (head == NULL) {
            head = stmt;
        } else {
            tail->next = stmt;
        }

        tail = stmt;

        while (tail->next != NULL) {
            tail = tail->next;
        }
    }

    expect(parser, TOK_RBRACE);

    return ast_block(head);
}

static Stmt *parse_let_statement(Parser *parser){
    char *name;
    Expr *value;
    Stmt *stmt;

    next_token(parser); /* let */

    if (parser->current.type != TOK_IDENT) {
        parser_error(
            parser,
            "expected variable name"
        );
    }

    name = strdup(parser->current.text);

    next_token(parser);

    expect(parser, TOK_ASSIGN);

    value = expression(parser);

    expect(parser, TOK_SEMI);

    stmt = ast_let(name, value);

    free(name);

    return stmt;
}

static Stmt *parse_print_statement(Parser *parser){
    Expr *expr;

    next_token(parser); /* print */

    expect(parser, TOK_LPAREN);

    expr = expression(parser);

    expect(parser, TOK_RPAREN);
    expect(parser, TOK_SEMI);

    return ast_print(expr);
}

static Stmt *parse_if_statement(Parser *parser){
    Expr *condition;
    Stmt *then_branch;
    Stmt *else_branch = NULL;

    next_token(parser); /* if */

    expect(parser, TOK_LPAREN);

    condition = expression(parser);

    expect(parser, TOK_RPAREN);

    then_branch = parse_block(parser);

    if (parser->current.type == TOK_ELSE) {
        next_token(parser);

        else_branch = parse_block(parser);
    }

    return ast_if(condition, then_branch, else_branch);
}

static Stmt *parse_while_statement(Parser *parser){
    Expr *condition;
    Stmt *body;

    next_token(parser); /* while */

    expect(parser, TOK_LPAREN);

    condition = expression(parser);

    expect(parser, TOK_RPAREN);

    body = parse_block(parser);

    return ast_while(condition, body);
}

static Stmt *parse_assignment_statement(Parser *parser){
    char *name = strdup(parser->current.text);
    Expr *value;
    Stmt *stmt;

    next_token(parser);

    expect(parser, TOK_ASSIGN);

    value = expression(parser);

    expect(parser, TOK_SEMI);

    stmt = ast_assign(name, value);

    free(name);

    return stmt;
}

static Stmt *statement(Parser *parser){
    switch (parser->current.type) {
        case TOK_LET:
            return parse_let_statement(parser);

        case TOK_PRINT:
            return parse_print_statement(parser);

        case TOK_IF:
            return parse_if_statement(parser);

        case TOK_WHILE:
            return parse_while_statement(parser);

        case TOK_LBRACE:
            return parse_block(parser);

        case TOK_IDENT:
            return parse_assignment_statement(parser);

        default:
            parser_error(
                parser,
                "expected statement"
            );

            return NULL;
    };
}

/* ------------------------------------------------------------------------- */
/* Public parser API                                                         */
/* ------------------------------------------------------------------------- */

void parser_init(Parser *parser, const char *source){
    lexer_init(
        &parser->lexer,
        source
    );

    parser->current = (Token){0};

    next_token(parser);
}

Stmt *parser_parse_program(Parser *parser){
    Stmt *head = NULL;
    Stmt *tail = NULL;

    while (parser->current.type != TOK_EOF) {
        Stmt *stmt = statement(parser);

        if (head == NULL) {
            head = stmt;
        } else {
            tail->next = stmt;
        }

        tail = stmt;

        while (tail->next != NULL) {
            tail = tail->next;
        }
    }

    return head;
}

void parser_destroy(Parser *parser) {
    token_free(&parser->current);
}
