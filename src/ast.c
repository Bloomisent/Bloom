#include "include/ast.h"
#include <stdlib.h>
#include <string.h>

/* ---------- allocation helpers ---------- */

static char *dupstr(const char *s){
    size_t n = strlen(s);
    char *p = malloc(n + 1);
    if (!p) exit(1);
    memcpy(p, s, n + 1);
    return p;
}

static Expr *new_expr(ExprKind kind){
    Expr *e = calloc(1, sizeof(*e));
    if (!e) exit(1);
    e->kind = kind;
    return e;
}

static Stmt *new_stmt(StmtKind kind){
    Stmt *s = calloc(1, sizeof(*s));
    if (!s) exit(1);
    s->kind = kind;
    return s;
}

/* ---------- expression constructors ---------- */

Expr *ast_int(long long value){
    Expr *e = new_expr(EX_INT);
    e->as.integer = value;
    e->type = TYPE_INT;
    return e;
}

Expr *ast_str(char* value){
    Expr *e = malloc(sizeof(*e));
    if (e == NULL){
        return NULL;
    };
    e->kind = EX_STR;
    e->as.string = value;
    e->type = TYPE_STR;
    return e;
}

Expr *ast_var(const char *name)
{
    Expr *e = new_expr(EX_VAR);
    e->as.var = dupstr(name);
    return e;
}

Expr *ast_binary(BinaryOp op, Expr *left, Expr *right)
{
    Expr *e = new_expr(EX_BINARY);
    e->as.binary.op    = op;
    e->as.binary.left  = left;
    e->as.binary.right = right;
    return e;
}

Expr *ast_unary(UnaryOp op, Expr *operand)
{
    Expr *e = new_expr(EX_UNARY);
    e->as.unary.op      = op;
    e->as.unary.operand = operand;
    return e;
}

/* ---------- statement constructors ---------- */

Stmt *ast_let(const char *name, Expr *value)
{
    Stmt *s = new_stmt(ST_LET);
    s->as.let.name  = dupstr(name);
    s->as.let.value = value;
    return s;
}

Stmt *ast_assign(const char *name, Expr *value)
{
    Stmt *s = new_stmt(ST_ASSIGN);
    s->as.assign.name  = dupstr(name);
    s->as.assign.value = value;
    return s;
}

Stmt *ast_print(Expr *value)
{
    Stmt *s = new_stmt(ST_PRINT);
    s->as.print = value;
    return s;
}

Stmt *ast_block(Stmt *body)
{
    Stmt *s = new_stmt(ST_BLOCK);
    s->as.block.body = body;
    return s;
}

Stmt *ast_if(Expr *condition, Stmt *then_branch, Stmt *else_branch)
{
    Stmt *s = new_stmt(ST_IF);
    s->as.if_stmt.condition   = condition;
    s->as.if_stmt.then_branch = then_branch;
    s->as.if_stmt.else_branch = else_branch;
    return s;
}

Stmt *ast_while(Expr *condition, Stmt *body)
{
    Stmt *s = new_stmt(ST_WHILE);
    s->as.while_stmt.condition = condition;
    s->as.while_stmt.body      = body;
    return s;
}

/* ---------- destructors ---------- */

void ast_free_expr(Expr *e)
{
    if (!e) return;

    switch (e->kind) {
        case EX_STR:
            free(e->as.string);
            break;
        case EX_VAR:
            free(e->as.var);
            break;
        case EX_BINARY:
            ast_free_expr(e->as.binary.left);
            ast_free_expr(e->as.binary.right);
            break;
        case EX_UNARY:
            ast_free_expr(e->as.unary.operand);
            break;
        default:
            break;
    }
    free(e);
}

/* Frees a whole statement list, following ->next. */
void ast_free_stmt(Stmt *s)
{
    while (s) {
        Stmt *next = s->next;

        switch (s->kind) {
            case ST_LET:
                free(s->as.let.name);
                ast_free_expr(s->as.let.value);
                break;
            case ST_ASSIGN:
                free(s->as.assign.name);
                ast_free_expr(s->as.assign.value);
                break;
            case ST_PRINT:
                ast_free_expr(s->as.print);
                break;
            case ST_BLOCK:
                ast_free_stmt(s->as.block.body);
                break;
            case ST_IF:
                ast_free_expr(s->as.if_stmt.condition);
                ast_free_stmt(s->as.if_stmt.then_branch);
                ast_free_stmt(s->as.if_stmt.else_branch);
                break;
            case ST_WHILE:
                ast_free_expr(s->as.while_stmt.condition);
                ast_free_stmt(s->as.while_stmt.body);
                break;
        }

        free(s);
        s = next;
    }
}
