.intel_syntax noprefix

.text
.global dot_product

dot_product:
        push    rbp
        mov     rbp, rsp

        mov     qword ptr[rbp-24], rdi
        mov     qword ptr[rbp-32], rsi
        mov     qword ptr[rbp-40], rdx

        pxor    xmm0, xmm0
        movss   dword ptr [rbp-4], xmm0
        mov     qword ptr[rbp-16], 0

        jmp     .end
.loop:
        mov     rax, qword ptr[rbp-16]
        lea     rdx, [0+rax*4]

        mov     rax, qword ptr[rbp-32]
        add     rax, rdx

        movss   xmm1, dword ptr [rax]
        mov     rax, qword ptr[rbp-16]

        lea     rdx, [0+rax*4]
        mov     rax, qword ptr[rbp-40]
        add     rax, rdx
        
        movss   xmm0, dword ptr [rax]
        mulss   xmm0, xmm1
        movss   xmm1, dword ptr [rbp-4]
        addss   xmm0, xmm1

        movss   dword ptr [rbp-4], xmm0
        add     qword ptr[rbp-16], 1
.end:
        mov     rax, qword ptr[rbp-16]
        cmp     rax, qword ptr[rbp-24]
        jb      .loop
    
        movss   xmm0, dword ptr [rbp-4]
        
        pop     rbp
        ret