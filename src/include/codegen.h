#ifndef TINYC_CODEGEN_H
#define TINYC_CODEGEN_H
#include "ast.h"
int codegen_nasm(Stmt *program, const char *output_path);
#endif
