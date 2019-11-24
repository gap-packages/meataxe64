/* pc6.s x86 assembler for big primes*/
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

/* pc6bma box x 21 x 73 64-bit prime HPMI */

/* input  rdi Afmt     rsi bwa     rdx  Cfmt      %rcx 2^90 % p*/

/* %rax,%rdx local working registers for multiply and other things */
/* %cl  counter to 73 to count middle loop
/* %ch  counter to 3 (unrolled 7) in inner loop */
/* %r9  Cfmt pointer running at all times */
/* %rdi Afmt pointer running at all times */
/* %rsi Bwa pointer running at all times */
/* %r8  2^90 % p */
/* %r10 0^38 1^26 constant mask for reduction */
/* %r11,%r12,%r13 accumulation (%r11 low order, %r13 high) */

	.text
	.globl	pc6bma
pc6bma:
        pushq   %r12
        pushq   %r13
        movq    %rdx,%r9
        movq    %rcx,%r8
        movq    $1,%r10     /* 0^38 1^26 */
        shlq    $26,%r10
        subq    $1,%r10
pc6bma1:                /* outer loop One row of A 73 cols of BC*/
        movzbq  0(%rdi),%rax
        cmpq    $255,%rax
        je      pc6bma9
        addq    $1,%rdi    /* Makes Afmt unaligned! */
        imulq   $1168,%rax,%rax
        addq    %rax,%r9
        mov     $73,%cl
pc6bma2:                /* middle loop round cols B cols C */
        movq    0(%r9),%r11
        movq    8(%r9),%r12
        xorq    %r13,%r13
        mov     $3,%ch  /* inner loop - unrolled * 7 */       
pc6bma3:                /* so three times round for 21 */
        movq    0(%rdi),%rax    /* 0 */
        mulq    0(%rsi)
        addq    %rax,%r11
        adcq    %rdx,%r12
        adcq    $0,%r13
        movq    8(%rdi),%rax    /* 1 */
        mulq    168(%rsi)
        addq    %rax,%r11
        adcq    %rdx,%r12
        adcq    $0,%r13
        movq    16(%rdi),%rax   /* 2 */
        mulq    336(%rsi)
        addq    %rax,%r11
        adcq    %rdx,%r12
        adcq    $0,%r13
        movq    24(%rdi),%rax   /* 3 */
        mulq    504(%rsi)
        addq    %rax,%r11
        adcq    %rdx,%r12
        adcq    $0,%r13
        movq    32(%rdi),%rax   /* 4 */
        mulq    672(%rsi)
        addq    %rax,%r11
        adcq    %rdx,%r12
        adcq    $0,%r13
        movq    40(%rdi),%rax   /* 5 */
        mulq    840(%rsi)
        addq    %rax,%r11
        adcq    %rdx,%r12
        adcq    $0,%r13
        movq    48(%rdi),%rax   /* 6 */
        mulq    1008(%rsi)
        addq    %rax,%r11
        adcq    %rdx,%r12
        adcq    $0,%r13
        addq    $56,%rdi        /* repeat that 3 times */
        addq    $1176,%rsi
        sub     $1,%ch
        jne     pc6bma3
/* reduce C back to two words and put back */
        shld    $38,%r12,%r13
        movq    %r13,%rax
        mulq    %r8              /* 2^90 mod p */
        andq    %r10,%r12
        addq    %rax,%r11
        adcq    %rdx,%r12
        movq    %r11,0(%r9)
        movq    %r12,8(%r9)
        addq    $16,%r9
        subq    $168,%rdi
        subq    $3520,%rsi   /* back all but one word */
        sub     $1,%cl
        jne     pc6bma2
                          /* middle loop ending */
        subq    $584,%rsi    /* back 73 words */
        addq    $168,%rdi
        jmp     pc6bma1   /* on to next chunk of Afmt */
pc6bma9:
        popq    %r13
        popq    %r12
        ret   

/* end of pc6.s */
