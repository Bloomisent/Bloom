#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/semantic.h"
#include "include/codegen.h"
#include "include/toolchain.h"

static char *read_file(const char *filename) {
    FILE *file;
    long size;
    char *buffer;
    file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf( stderr, "Error: could not open '%s'\n", filename );
        return NULL;
    };
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = malloc((size_t)size + 1);
    if (buffer == NULL) {
        fprintf( stderr, "Error: out of memory\n" );
        fclose(file);
        return NULL;
    };
    fread( buffer, 1, (size_t)size, file );
    buffer[size] = '\0';
    fclose(file);
    return buffer;
};

static void print_usage(const char *program){
    printf(
        "Bloom compiler\n\n"
        "Usage:\n"
        "    %s <source.bloom> <output.exe>\n\n"
        "Example:\n"
        "    %s hello.bloom hello.exe\n",
        program,
        program
    );
}

int main(int argc, char **argv){
    const char *source_file;
    const char *output_file;

    char asm_file[1024];
    char obj_file[1024];

    char *source;
    Parser parser;
    Stmt *program;

    if (argc != 3) {
        print_usage(argv[0]);

        return 1;
    }

    source_file = argv[1];
    output_file = argv[2];

    snprintf(
        asm_file,
        sizeof(asm_file),
        "%s.asm",
        output_file
    );

    snprintf(
        obj_file,
        sizeof(obj_file),
        "%s.obj",
        output_file
    );

    printf("Reading %s...\n", source_file);

    source = read_file(source_file);

    if (source == NULL) {
        return 1;
    };

    printf("Parsing...\n");

    parser_init(&parser, source);

    program = parser_parse_program(&parser);
    parser_destroy(&parser);

    printf("Checking...\n");

    if (!semantic_check(program)) {
        fprintf(
            stderr,
            "Compilation failed during semantic analysis.\n"
        );

        free(source);
        ast_free_stmt(program);

        return 1;
    };

    printf("Generating NASM...\n");

    if (!codegen_nasm(program, asm_file)) {
        fprintf(
            stderr,
            "Failed to generate assembly.\n"
        );

        free(source);
        ast_free_stmt(program);

        return 1;
    };

    if (!compile(asm_file, obj_file, output_file)) {
        fprintf(
            stderr,
            "Compilation failed during the toolchain stage.\n"
        );

        free(source);
        ast_free_stmt(program);

        return 1;
    };

    free(source);

    ast_free_stmt(program);

    printf("\nDone!\nExecutable: %s\n", output_file);

    return 0;
}
