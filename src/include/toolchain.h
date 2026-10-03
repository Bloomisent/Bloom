#pragma once

int assemble(
    const char *asm_file,
    const char *obj_file
);

int link_object(
    const char *obj_file,
    const char *exe_file
);

int compile(
    const char *asm_file,
    const char *obj_file,
    const char *exe_file
);
