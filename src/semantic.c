#include "include/semantic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Symbol list                                                               */
/* ------------------------------------------------------------------------- */

typedef struct Name {
    char *name;
    struct Name *next;
} Name;

static int is_declared(Name *names, const char *name){
    for (Name *current = names; current != NULL;current = current->next) {
        if (strcmp(current->name, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static void declare_name(Name **names, const char *name){
    Name *entry = malloc(sizeof(*entry));

    entry->name = strdup(name);
    entry->next = *names;

    *names = entry;
}

static void free_names(Name *names){
    while (names != NULL) {
        Name *next = names->next;

        free(names->name);
        free(names);

        names = next;
    }
}

/* ------------------------------------------------------------------------- */
/* Expression checking                                                       */
/* ------------------------------------------------------------------------- */

static int check_expression(Expr *expr, Name *names){
    if (expr == NULL) {
        return 1;
    }

    switch (expr->kind) {
        case EX_INT:
            return 1;

        case EX_STR:
            return 1;

        case EX_VAR:
            if (!is_declared(names, expr->as.var)) {
                fprintf(
                    stderr,
                    "Semantic error: variable '%s' used before declaration\n",
                    expr->as.var
                );

                return 0;
            }

            return 1;

        case EX_UNARY:
            return check_expression(expr->as.unary.operand, names);

        case EX_BINARY:
            if (!check_expression(expr->as.binary.left,names)) {
                return 0;
            }

            return check_expression(expr->as.binary.right, names);
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
/* Statement checking                                                        */
/* ------------------------------------------------------------------------- */

static int check_statements(Stmt *stmt, Name **names){
    for (; stmt != NULL; stmt = stmt->next) {
        switch (stmt->kind) {

            case ST_LET:
                if (is_declared(*names, stmt->as.let.name)) {
                    fprintf(
                        stderr,
                        "Semantic error: variable '%s' "
                        "already declared\n",
                        stmt->as.let.name
                    );

                    return 0;
                }

                if (!check_expression(stmt->as.let.value,*names)) {
                    return 0;
                }

                declare_name(names, stmt->as.let.name);
                break;

            case ST_ASSIGN:
                if (!is_declared(*names, stmt->as.assign.name)) {
                    fprintf(
                        stderr,
                        "Semantic error: assignment to "
                        "undeclared variable '%s'\n",
                        stmt->as.assign.name
                    );

                    return 0;
                }

                if (!check_expression(stmt->as.assign.value, *names)) {
                    return 0;
                }

                break;

            case ST_PRINT:
                if (!check_expression(stmt->as.print, *names)) {
                    return 0;
                }

                break;

            case ST_BLOCK:
                if (!check_statements(stmt->as.block.body, names)) {
                    return 0;
                }

                break;

            case ST_IF:
                if (!check_expression(stmt->as.if_stmt.condition, *names)) {
                    return 0;
                };

                if (!check_statements(stmt->as.if_stmt.then_branch, names)) {
                    return 0;
                };

                if (stmt->as.if_stmt.else_branch != NULL && !check_statements(stmt->as.if_stmt.else_branch, names)) {
                    return 0;
                };

                break;

            case ST_WHILE:
                if (!check_expression(stmt->as.while_stmt.condition, *names)) {
                    return 0;
                };

                if (!check_statements(stmt->as.while_stmt.body, names)) {
                    return 0;
                };

                break;
        }
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
/* Public semantic-analysis API                                             */
/* ------------------------------------------------------------------------- */

int semantic_check(Stmt *program){
    Name *names = NULL;

    int result = check_statements(program, &names);
    free_names(names);

    return result;
}
