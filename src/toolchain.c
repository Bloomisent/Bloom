#include "include/toolchain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define NASM_COMMAND "nasm"
#define LINKER_COMMAND "gcc"

static int run_command(const char *command){
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;

    char command_line[4096];

    memset(&startup_info, 0, sizeof(startup_info));
    memset(&process_info, 0, sizeof(process_info));

    startup_info.cb = sizeof(startup_info);

    snprintf(command_line, sizeof(command_line), "%s", command);

    printf("Running: %s\n", command_line);

    if (!CreateProcessA(NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info)) {
        fprintf(
            stderr,
            "Failed to start command:\n%s\n",
            command
        );

        fprintf(
            stderr,
            "Windows error: %lu\n",
            GetLastError()
        );

        return -1;
    }

    WaitForSingleObject(process_info.hProcess,INFINITE);

    DWORD exit_code = 0;

    if (!GetExitCodeProcess(process_info.hProcess,&exit_code)) {
        fprintf(
            stderr,
            "Failed to get process exit code.\n"
        );

        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);

        return -1;
    }

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    return (int)exit_code;
}

int assemble(const char *asm_file, const char *obj_file){ // NASM
    char command[4096];

    snprintf(
        command,
        sizeof(command),
        "%s -f win64 \"%s\" -o \"%s\"",
        NASM_COMMAND,
        asm_file,
        obj_file
    );

    printf("\n[1/2] Assembling...\n");

    int result = run_command(command);

    if (result != 0) {
        fprintf(
            stderr,
            "NASM failed with exit code %d.\n",
            result
        );

        return 0;
    }

    return 1;
}

int link_object(const char *obj_file,const char *exe_file){
    char command[4096];

    snprintf(
        command,
        sizeof(command),
        "%s \"%s\" -o \"%s\"",
        LINKER_COMMAND,
        obj_file,
        exe_file
    );

    printf(
        "\n[2/2] Linking...\n"
    );

    int result = run_command(command);

    if (result != 0) {
        fprintf(
            stderr,
            "Linker failed with exit code %d.\n",
            result
        );

        return 0;
    }

    return 1;
}

int compile(const char *asm_file, const char *obj_file, const char *exe_file){
    printf(
        "========================================\n"
        " Bloom Toolchain\n"
        "========================================\n"
    );

    /*
     Assemble the NASM source into a
     Windows x64 object file.
    */

    if (!assemble(asm_file,obj_file)) {
        return 0;
    }

    /*
     Link the object file into the
     final Windows executable.
    */

    if (!link_object(obj_file,exe_file)) {
        return 0;
    }

    printf("\nCompilation successful!\nOutput: %s\n", exe_file);

    return 1;
}
