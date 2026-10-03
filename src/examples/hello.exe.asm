default rel
extern printf
section .data
fmt_int db "%lld", 13, 10, 0
section .text
global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 4096
    mov rax, 1
    mov [rbp-8], rax
.L0:
    mov rax, [rbp-8]
    push rax
    mov rax, 10
    mov rcx, rax
    pop rax
    cmp rax, rcx
    setle al
    movzx rax, al
    cmp rax, 0
    je .L1
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-8]
    mov rcx, rax
    pop rax
    imul rax, rcx
    mov rdx, rax
    lea rcx, [rel fmt_int]
    sub rsp, 40
    xor eax, eax
    call printf
    add rsp, 40
    mov rax, [rbp-8]
    push rax
    mov rax, 1
    mov rcx, rax
    pop rax
    add rax, rcx
    mov [rbp-8], rax
    jmp .L0
.L1:
    mov rax, [rbp-8]
    push rax
    mov rax, 11
    mov rcx, rax
    pop rax
    cmp rax, rcx
    sete al
    movzx rax, al
    cmp rax, 0
    je .L2
    mov rax, 1234
    mov rdx, rax
    lea rcx, [rel fmt_int]
    sub rsp, 40
    xor eax, eax
    call printf
    add rsp, 40
    jmp .L3
.L2:
    mov rax, 0
    mov rdx, rax
    lea rcx, [rel fmt_int]
    sub rsp, 40
    xor eax, eax
    call printf
    add rsp, 40
.L3:
    xor eax, eax
    leave
    ret
