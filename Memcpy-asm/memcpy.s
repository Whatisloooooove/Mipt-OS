.intel_syntax noprefix

.text
.global my_memcpy

my_memcpy:
    mov     rax, rdi
    test    edx, edx
    je      .end
    xor     rcx, rcx

.loop:
    cmp     edx, 8
    jb      .partial_copy
    
    mov     r8, qword ptr [rsi + rcx]
    mov     qword ptr [rax + rcx], r8
    
    add     rcx, 8
    sub     edx, 8
    jae     .loop

.partial_copy:
    test    edx, edx
    jz      .end
    
    mov     r8b, byte ptr [rsi + rcx]
    mov     byte ptr [rax + rcx], r8b
    
    inc     rcx
    dec     edx
    jnz     .partial_copy

.end:
    ret
