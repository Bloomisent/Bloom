default rel
extern printf
section .data
fmt_int db "%lld", 13, 10, 0
section .text
global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    lea rax, [rel str0]
    mov rdx, rax
    lea rcx, [rel fmt_int]
    call printf
    xor eax, eax
    leave
    ret
