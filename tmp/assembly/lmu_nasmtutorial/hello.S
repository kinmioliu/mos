; this is a comment
    global _start

    section .text
_start: mov rax, 1
1:
    mov 
    mov rdi, 1
    mov rsi, message
    mov rdx, 13
    syscall
    mov rax, 60
    xor rdi, rdi
    syscall

    section .data
message: db "hello, world",10
