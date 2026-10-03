#ifndef TINYC_AST_H
#define TINYC_AST_H
#include <stddef.h>

typedef enum { TYPE_INT, TYPE_STR } Type;
typedef enum { EX_INT, EX_STR, EX_VAR, EX_BINARY, EX_UNARY } ExprKind;
typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE } BinaryOp;
typedef enum { UOP_NEG, UOP_NOT } UnaryOp;

typedef struct Expr Expr;
struct Expr {
    ExprKind kind;
    Type type;
    union {
        char* string;
        long long integer;
        char *var;
        struct { BinaryOp op; Expr *left; Expr *right; } binary;
        struct { UnaryOp op; Expr *operand; } unary;
    } as;
};

typedef enum { ST_LET, ST_ASSIGN, ST_PRINT, ST_BLOCK, ST_IF, ST_WHILE } StmtKind;
typedef struct Stmt Stmt;
struct Stmt {
    StmtKind kind;
    Stmt *next;
    union {
        struct { char *name; Expr *value; } let;
        struct { char *name; Expr *value; } assign;
        Expr *print;
        struct { Stmt *body; } block;
        struct { Expr *condition; Stmt *then_branch; Stmt *else_branch; } if_stmt;
        struct { Expr *condition; Stmt *body; } while_stmt;
    } as;
};

Expr *ast_int(long long value);
Expr *ast_str(char* value);
Expr *ast_var(const char *name);
Expr *ast_binary(BinaryOp op, Expr *left, Expr *right);
Expr *ast_unary(UnaryOp op, Expr *operand);
Stmt *ast_let(const char *name, Expr *value);
Stmt *ast_assign(const char *name, Expr *value);
Stmt *ast_print(Expr *expr);
Stmt *ast_block(Stmt *body);
Stmt *ast_if(Expr *condition, Stmt *then_branch, Stmt *else_branch);
Stmt *ast_while(Expr *condition, Stmt *body);
void ast_free_expr(Expr *e);
void ast_free_stmt(Stmt *s);

#endif
