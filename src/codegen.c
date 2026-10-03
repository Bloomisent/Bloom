#include "include/codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Code-generator state                                                      */
/* ------------------------------------------------------------------------- */

typedef struct StringLiteral {
    char *value;
    int label;

    struct StringLiteral *next;
} StringLiteral;

typedef struct Var {
    char *name;
    int offset;
    struct Var *next;
} Var;

typedef struct {
    FILE *out;
    Var *vars;

    StringLiteral *strings;

    int next_offset;
    int label;
    int next_string;
} CodeGenerator;

/* ------------------------------------------------------------------------- */
/* Variables and labels                                                      */
/* ------------------------------------------------------------------------- */

static int lookup_variable(CodeGenerator *cg, const char *name){
    for (Var *var = cg->vars; var != NULL; var = var->next) {
        if (strcmp(var->name, name) == 0) {
            return var->offset;
        }
    }

    return 0;
}

static int add_variable(CodeGenerator *cg, const char *name){
    int existing_offset = lookup_variable(cg, name);

    if (existing_offset != 0) {
        return existing_offset;
    }

    Var *var = calloc(
        1,
        sizeof(*var)
    );

    var->name = strdup(name);

    cg->next_offset += 8;
    var->offset = cg->next_offset;

    var->next = cg->vars;
    cg->vars = var;

    return var->offset;
}

static int add_string(CodeGenerator *cg, const char *value){
    StringLiteral *string = malloc(
        sizeof(*string)
    );

    string->value = strdup(value);

    string->label = cg->next_string++;

    string->next = cg->strings;

    cg->strings = string;

    return string->label;
}

static int new_label(CodeGenerator *cg){
    return cg->label++;
}

static void free_variables(Var *vars){
    while (vars != NULL) {
        Var *next = vars->next;

        free(vars->name);
        free(vars);

        vars = next;
    }
}

/* ------------------------------------------------------------------------- */
/* Expression generation                                                     */
/* ------------------------------------------------------------------------- */

static void generate_expression(CodeGenerator *cg, Expr *expr){
    switch (expr->kind) {

        case EX_INT:
            fprintf(
                cg->out,
                "    mov rax, %lld\n",
                expr->as.integer
            );

            return;

        case EX_STR:
            int label = add_string(
                cg,
                expr->as.string
            );

            fprintf(
                cg->out,
                "    lea rax, [rel str%d]\n",
                label
            );

            return;

        case EX_VAR:
            fprintf(
                cg->out,
                "    mov rax, [rbp-%d]\n",
                lookup_variable(cg, expr->as.var)
            );

            return;

        case EX_UNARY:
            generate_expression(cg, expr->as.unary.operand);

            if (expr->as.unary.op == UOP_NEG) {
                fprintf(
                    cg->out,
                    "    neg rax\n"
                );
            }
            else {
                fprintf(
                    cg->out,
                    "    cmp rax, 0\n"
                    "    sete al\n"
                    "    movzx rax, al\n"
                );
            }

            return;

        case EX_BINARY:
            break;
    }

    /*
     * Generate the left side first.
     *
     * RAX:
     *     left result
     *
     * The result is pushed onto the stack while
     * the right side is generated.
     */

    generate_expression(cg, expr->as.binary.left);

    fprintf(
        cg->out,
        "    push rax\n"
    );

    generate_expression(cg, expr->as.binary.right);

    /*
     * Right side is currently in RAX.
     * Move it to RCX, then restore the left
     * side into RAX.
     */

    fprintf(
        cg->out,
        "    mov rcx, rax\n"
        "    pop rax\n"
    );

    switch (expr->as.binary.op) {

        case OP_ADD:
            fprintf(
                cg->out,
                "    add rax, rcx\n"
            );
            break;

        case OP_SUB:
            fprintf(
                cg->out,
                "    sub rax, rcx\n"
            );
            break;

        case OP_MUL:
            fprintf(
                cg->out,
                "    imul rax, rcx\n"
            );
            break;

        case OP_DIV:
            fprintf(
                cg->out,
                "    cqo\n"
                "    idiv rcx\n"
            );
            break;

        case OP_MOD:
            fprintf(
                cg->out,
                "    cqo\n"
                "    idiv rcx\n"
                "    mov rax, rdx\n"
            );
            break;

        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:

            fprintf(
                cg->out,
                "    cmp rax, rcx\n"
            );

            switch (expr->as.binary.op) {

                case OP_EQ:
                    fprintf(
                        cg->out,
                        "    sete al\n"
                    );
                    break;

                case OP_NE:
                    fprintf(
                        cg->out,
                        "    setne al\n"
                    );
                    break;

                case OP_LT:
                    fprintf(
                        cg->out,
                        "    setl al\n"
                    );
                    break;

                case OP_LE:
                    fprintf(
                        cg->out,
                        "    setle al\n"
                    );
                    break;

                case OP_GT:
                    fprintf(
                        cg->out,
                        "    setg al\n"
                    );
                    break;

                case OP_GE:
                    fprintf(
                        cg->out,
                        "    setge al\n"
                    );
                    break;

                default:
                    break;
            }

            fprintf(
                cg->out,
                "    movzx rax, al\n"
            );

            break;

        default:
            break;
    }
};

/* ------------------------------------------------------------------------- */
/* Statement generation                                                      */
/* ------------------------------------------------------------------------- */

static void generate_statements(CodeGenerator *cg, Stmt *stmt){
    for (; stmt != NULL; stmt = stmt->next) {
        switch (stmt->kind) {

            case ST_LET: {
                int offset = add_variable(cg, stmt->as.let.name);
                generate_expression(cg, stmt->as.let.value);

                fprintf(
                    cg->out,
                    "    mov [rbp-%d], rax\n",
                    offset
                );

                break;
            }

            case ST_ASSIGN: {
                int offset = lookup_variable(cg, stmt->as.assign.name);
                generate_expression(cg, stmt->as.assign.value);

                fprintf(
                    cg->out,
                    "    mov [rbp-%d], rax\n",
                    offset
                );

                break;
            }

            case ST_PRINT:
                generate_expression(cg, stmt->as.print);

                fprintf(
                    cg->out,
                    "    mov rdx, rax\n"
                    "    lea rcx, [rel fmt_int]\n"
                    "    call printf\n"
                );

                break;

            case ST_BLOCK:
                generate_statements(cg, stmt->as.block.body);

                break;

            case ST_IF: {
                int else_label = new_label(cg);
                int end_label = new_label(cg);

                generate_expression(cg, stmt->as.if_stmt.condition);

                fprintf(
                    cg->out,
                    "    cmp rax, 0\n"
                    "    je .L%d\n",
                    else_label
                );

                generate_statements(cg, stmt->as.if_stmt.then_branch);

                fprintf(
                    cg->out,
                    "    jmp .L%d\n"
                    ".L%d:\n",
                    end_label,
                    else_label
                );

                if (stmt->as.if_stmt.else_branch != NULL) {
                    generate_statements(cg, stmt->as.if_stmt.else_branch);
                }

                fprintf(
                    cg->out,
                    ".L%d:\n",
                    end_label
                );

                break;
            }

            case ST_WHILE: {
                int start_label = new_label(cg);
                int end_label = new_label(cg);

                fprintf(
                    cg->out,
                    ".L%d:\n",
                    start_label
                );

                generate_expression(cg, stmt->as.while_stmt.condition);

                fprintf(
                    cg->out,
                    "    cmp rax, 0\n"
                    "    je .L%d\n",
                    end_label
                );

                generate_statements(cg, stmt->as.while_stmt.body);

                fprintf(
                    cg->out,
                    "    jmp .L%d\n"
                    ".L%d:\n",
                    start_label,
                    end_label
                );

                break;
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Public code-generation API                                                */
/* ------------------------------------------------------------------------- */

int codegen_nasm(Stmt *program, const char *path){
    FILE *file = fopen(
        path,
        "w"
    );

    if (file == NULL) {
        perror(path);
        return 0;
    }

    CodeGenerator cg = {0};

    cg.out = file;

    fprintf(
        file,
        "default rel\n"
        "extern printf\n"

        "section .data\n"
        "fmt_int db \"%%lld\", 13, 10, 0\n"

        "section .text\n"
        "global main\n"

        "main:\n"
        "    push rbp\n"
        "    mov rbp, rsp\n"
        "    sub rsp, 32\n"
    );

    generate_statements(&cg, program);

    fprintf(
        file,
        "    xor eax, eax\n"
        "    leave\n"
        "    ret\n"
    );

    fclose(file);

    free_variables(cg.vars);

    return 1;
}
