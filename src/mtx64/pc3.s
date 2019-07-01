/* pcrits.s x86 assembler before AVX-512 changes*/
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

// pc3aca  rdi=prog rsi=bwa rdx stride
// %rax program byte
// r8 running program counter, %r9 destination r10 slices left

.text
	.globl	pc3aca
pc3aca: 
        movq        $3,%r10    /* number of slices */
pc3aca1:                        /* next slice */
        movq        %rsi,%r9
        addq        $256,%r9   /* destination starts slot 2! */
        movq        %rdi,%r8   /* restart program from beginning */
        movdqa -128(%r9),%xmm0  /* load accumulator from slot 1 */
        movdqa -112(%r9),%xmm1
        movdqa -96(%r9),%xmm2
        movdqa -80(%r9),%xmm3
        movdqa -64(%r9),%xmm4
        movdqa -48(%r9),%xmm5
        movdqa -32(%r9),%xmm6
        movdqa -16(%r9),%xmm7
pc3aca2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to second program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pc3aca4       /* no - go see what it is */
pc3aca3:
        shlq    $7,%rax       /* convert to displacement */

	movdqa	(%rax,%rsi),%xmm8
	movdqa	64(%rax,%rsi),%xmm9
        pxor    %xmm0,%xmm8      /*  bu^au -> au   */
        pxor    %xmm9,%xmm4      /*  at^bt -> bt   */
        pxor    %xmm8,%xmm9      /*  au^at -> at   */
        pxor    %xmm4,%xmm0      /*  bt^bu -> bu   */
        por     %xmm4,%xmm9      /*  bt|at -> at   */
        por     %xmm0,%xmm8      /*  bu|au -> au   */
        movdqa  %xmm9,%xmm0      /*  at    -> bu   */
        movdqa  %xmm8,%xmm4      /*  au    -> bt   */
        movdqa  %xmm0,(%r9)
        movdqa  %xmm4,64(%r9)

	movdqa	16(%rax,%rsi),%xmm8
	movdqa	80(%rax,%rsi),%xmm9
        pxor    %xmm1,%xmm8      /*  bu^au -> au   */
        pxor    %xmm9,%xmm5      /*  at^bt -> bt   */
        pxor    %xmm8,%xmm9      /*  au^at -> at   */
        pxor    %xmm5,%xmm1      /*  bt^bu -> bu   */
        por     %xmm5,%xmm9      /*  bt|at -> at   */
        por     %xmm1,%xmm8      /*  bu|au -> au   */
        movdqa  %xmm9,%xmm1      /*  at    -> bu   */
        movdqa  %xmm8,%xmm5      /*  au    -> bt   */
        movdqa  %xmm1,16(%r9)
        movdqa  %xmm5,80(%r9)

	movdqa	32(%rax,%rsi),%xmm8
	movdqa	96(%rax,%rsi),%xmm9
        pxor    %xmm2,%xmm8      /*  bu^au -> au   */
        pxor    %xmm9,%xmm6      /*  at^bt -> bt   */
        pxor    %xmm8,%xmm9      /*  au^at -> at   */
        pxor    %xmm6,%xmm2      /*  bt^bu -> bu   */
        por     %xmm6,%xmm9      /*  bt|at -> at   */
        por     %xmm2,%xmm8      /*  bu|au -> au   */
        movdqa  %xmm9,%xmm2      /*  at    -> bu   */
        movdqa  %xmm8,%xmm6      /*  au    -> bt   */
        movdqa  %xmm2,32(%r9)
        movdqa  %xmm6,96(%r9)

	movdqa	48(%rax,%rsi),%xmm8
	movdqa	112(%rax,%rsi),%xmm9
        pxor    %xmm3,%xmm8      /*  bu^au -> au   */
        pxor    %xmm9,%xmm7      /*  at^bt -> bt   */
        pxor    %xmm8,%xmm9      /*  au^at -> at   */
        pxor    %xmm7,%xmm3      /*  bt^bu -> bu   */
        por     %xmm7,%xmm9      /*  bt|at -> at   */
        por     %xmm3,%xmm8      /*  bu|au -> au   */
        movdqa  %xmm9,%xmm3      /*  at    -> bu   */
        movdqa  %xmm8,%xmm7      /*  au    -> bt   */
        movdqa  %xmm3,48(%r9)
        movdqa  %xmm7,112(%r9)

        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pc3aca3      /* yes - go straight round again */
pc3aca4:
        cmpq    $159,%rax
        ja      pc3aca5      /* not a load of accumulator either */
        subq    $80,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movdqa  0(%rax,%rsi),%xmm0
        movdqa  16(%rax,%rsi),%xmm1
        movdqa  32(%rax,%rsi),%xmm2
        movdqa  48(%rax,%rsi),%xmm3
        movdqa  64(%rax,%rsi),%xmm4
        movdqa  80(%rax,%rsi),%xmm5
        movdqa  96(%rax,%rsi),%xmm6
        movdqa  112(%rax,%rsi),%xmm7
        jmp     pc3aca2
pc3aca5:
        cmpq    $239,%rax
        ja      pc3aca6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pc3aca2

pc3aca6:                      /* anything 240+ is stop at the moment */
        addq    %rdx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pc3aca1
        ret

// end of pc3aca

// pc3acj  rdi=prog rsi=bwa rdx stride
// %rax program byte
// r8 running program counter, %r9 destination r10 slices left

.text
	.globl	pc3acj
pc3acj: 
        movq        $3,%r10    /* number of slices */
pc3acj1:                        /* next slice */
        movq        %rsi,%r9
        addq        $256,%r9   /* destination starts slot 2! */
        movq        %rdi,%r8   /* restart program from beginning */
        vmovdqa -128(%r9),%ymm0  /* load accumulator from slot 1 */
        vmovdqa -96(%r9),%ymm1
        vmovdqa -64(%r9),%ymm2
        vmovdqa -32(%r9),%ymm3
pc3acj2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to second program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pc3acj4       /* no - go see what it is */
pc3acj3:
        shlq    $7,%rax       /* convert to displacement */

	vmovdqa	(%rax,%rsi),%ymm4
	vmovdqa	64(%rax,%rsi),%ymm5
        vpxor    %ymm0,%ymm4,%ymm4      /*  bu^au -> au   */
        vpxor    %ymm5,%ymm2,%ymm2      /*  at^bt -> bt   */
        vpxor    %ymm4,%ymm5,%ymm5      /*  au^at -> at   */
        vpxor    %ymm2,%ymm0,%ymm0      /*  bt^bu -> bu   */
        vpor     %ymm2,%ymm5,%ymm5      /*  bt|at -> at   */
        vpor     %ymm0,%ymm4,%ymm4      /*  bu|au -> au   */
        vmovdqa  %ymm5,%ymm0            /*  at    -> bu   */
        vmovdqa  %ymm4,%ymm2            /*  au    -> bt   */
        vmovdqa  %ymm0,(%r9)
        vmovdqa  %ymm2,64(%r9)

	vmovdqa	32(%rax,%rsi),%ymm4
	vmovdqa	96(%rax,%rsi),%ymm5
        vpxor    %ymm1,%ymm4,%ymm4      /*  bu^au -> au   */
        vpxor    %ymm5,%ymm3,%ymm3      /*  at^bt -> bt   */
        vpxor    %ymm4,%ymm5,%ymm5      /*  au^at -> at   */
        vpxor    %ymm3,%ymm1,%ymm1      /*  bt^bu -> bu   */
        vpor     %ymm3,%ymm5,%ymm5      /*  bt|at -> at   */
        vpor     %ymm1,%ymm4,%ymm4      /*  bu|au -> au   */
        vmovdqa  %ymm5,%ymm1            /*  at    -> bu   */
        vmovdqa  %ymm4,%ymm3            /*  au    -> bt   */
        vmovdqa  %ymm1,32(%r9)
        vmovdqa  %ymm3,96(%r9)

        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pc3acj3      /* yes - go straight round again */
pc3acj4:
        cmpq    $159,%rax
        ja      pc3acj5      /* not a load of accumulator either */
        subq    $80,%rax
        shlq    $7,%rax        /* multiply by slot size */
        vmovdqa  0(%rax,%rsi),%ymm0
        vmovdqa  32(%rax,%rsi),%ymm1
        vmovdqa  64(%rax,%rsi),%ymm2
        vmovdqa  96(%rax,%rsi),%ymm3
        jmp     pc3acj2
pc3acj5:
        cmpq    $239,%rax
        ja      pc3acj6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pc3acj2

pc3acj6:                      /* anything 240+ is stop at the moment */
        addq    %rdx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pc3acj1
        ret

// end of pc3acj


// pc3acj  rdi=prog rsi=bwa rdx stride
// %rax program byte
// r8 running program counter, %r9 destination r10 slices left

.text
	.globl	pc3acm
pc3acm: 
        movq        $3,%r10    /* number of slices */
pc3acm1:                        /* next slice */
        movq        %rsi,%r9
        addq        $256,%r9   /* destination starts slot 2! */
        movq        %rdi,%r8   /* restart program from beginning */
        vmovdqa64 -128(%r9),%zmm0  /* load accumulator from slot 1 */
        vmovdqa64 -64(%r9),%zmm1
pc3acm2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to second program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pc3acm4       /* no - go see what it is */
pc3acm3:
        shlq    $7,%rax       /* convert to displacement */

	vmovdqa64  (%rax,%rsi),%zmm2
	vmovdqa64  64(%rax,%rsi),%zmm3
        vpxorq    %zmm0,%zmm2,%zmm2      /*  bu^au -> au   */
        vpxorq    %zmm3,%zmm1,%zmm1      /*  at^bt -> bt   */
        vpxorq    %zmm2,%zmm3,%zmm3      /*  au^at -> at   */
        vpxorq    %zmm1,%zmm0,%zmm0      /*  bt^bu -> bu   */
        vporq     %zmm1,%zmm3,%zmm3      /*  bt|at -> at   */
        vporq     %zmm0,%zmm2,%zmm2      /*  bu|au -> au   */
        vmovdqa64  %zmm3,%zmm0            /*  at    -> bu   */
        vmovdqa64  %zmm2,%zmm1            /*  au    -> bt   */
        vmovdqa64  %zmm0,(%r9)
        vmovdqa64  %zmm2,64(%r9)

        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pc3acm3      /* yes - go straight round again */
pc3acm4:
        cmpq    $159,%rax
        ja      pc3acm5      /* not a load of accumulator either */
        subq    $80,%rax
        shlq    $7,%rax        /* multiply by slot size */
        vmovdqa64  0(%rax,%rsi),%zmm0
        vmovdqa64  64(%rax,%rsi),%zmm1
        jmp     pc3acm2
pc3acm5:
        cmpq    $239,%rax
        ja      pc3acm6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pc3acm2

pc3acm6:                      /* anything 240+ is stop at the moment */
        addq    %rdx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pc3acm1
        ret

// end of pc3acm


/* input  rdi Afmt     rsi bwa     rdx  Cfmt  */

/* six registers point in bwa  0   rax   rbx  */
/*                             1   rbp   r9   */
/*                             2   rcx   r10  */
	.text
	.globl	pc3bma
pc3bma:
        pushq   %rbx
        pushq   %rbp
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pc3bma5        /* yes - return           */
pc3bma2:
	salq	$7, %rbx      /* 128 * byte Afmt        */
        addq    %rbx,%rdx     /* skip some rows of Cfmt */

        movq    %rcx,%rax    /*  adslice 0  */
        movq    %rcx,%rbp    /*  adslice 1  */
/*      movq    %rcx,%rcx    /*  adslice 2  */
        sarq    $2,%rax      /*  shift slice 0*/
        sarq    $10,%rbp     /*  shift slice 1*/
        sarq    $18,%rcx     /*  shift slice 2*/
        andq    $8128,%rax   /*  and slice 0*/
        andq    $8128,%rbp   /*  and slice 1  */   
        andq    $8128,%rcx   /*  and slice 2*/
        movq    %rax,%rbx    /*  adslice 0+ */
        movq    %rbp,%r9     /*  adslice 1+ */
        movq    %rcx,%r10    /*  adslice 2+ */
        xorq    $64,%rbx
        xorq    $64,%r9
        xorq    $64,%r10
        addq    $4,%rdi       /* point to next Afmt word*/

	movdqa	(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	movdqa	64(%rdx),%xmm1
        movdqa  (%rsi,%rax),%xmm2
        movdqa  (%rsi,%rbx),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  5248(%rsi,%rbp),%xmm0
        movdqa  5248(%rsi,%r9),%xmm1
        pxor    %xmm3,%xmm0
        pxor    %xmm1,%xmm2
        pxor    %xmm0,%xmm1
        pxor    %xmm2,%xmm3
        por     %xmm1,%xmm2
        por     %xmm0,%xmm3
        movdqa  10496(%rsi,%rcx),%xmm1
        movdqa  10496(%rsi,%r10),%xmm0
        pxor    %xmm2,%xmm1
        pxor    %xmm0,%xmm3
        pxor    %xmm1,%xmm0
        pxor    %xmm3,%xmm2
        por     %xmm3,%xmm0
        por     %xmm2,%xmm1
        movdqa  %xmm0,(%rdx)
        movdqa  %xmm1,64(%rdx)

	movdqa	16(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	movdqa	80(%rdx),%xmm1
        movdqa  16(%rsi,%rax),%xmm2
        movdqa  16(%rsi,%rbx),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  5264(%rsi,%rbp),%xmm0
        movdqa  5264(%rsi,%r9),%xmm1
        pxor    %xmm3,%xmm0
        pxor    %xmm1,%xmm2
        pxor    %xmm0,%xmm1
        pxor    %xmm2,%xmm3
        por     %xmm1,%xmm2
        por     %xmm0,%xmm3
        movdqa  10512(%rsi,%rcx),%xmm1
        movdqa  10512(%rsi,%r10),%xmm0
        pxor    %xmm2,%xmm1
        pxor    %xmm0,%xmm3
        pxor    %xmm1,%xmm0
        pxor    %xmm3,%xmm2
        por     %xmm3,%xmm0
        por     %xmm2,%xmm1
        movdqa  %xmm0,16(%rdx)
        movdqa  %xmm1,80(%rdx)

	movdqa	32(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	movdqa	96(%rdx),%xmm1
        movdqa  32(%rsi,%rax),%xmm2
        movdqa  32(%rsi,%rbx),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  5280(%rsi,%rbp),%xmm0
        movdqa  5280(%rsi,%r9),%xmm1
        pxor    %xmm3,%xmm0
        pxor    %xmm1,%xmm2
        pxor    %xmm0,%xmm1
        pxor    %xmm2,%xmm3
        por     %xmm1,%xmm2
        por     %xmm0,%xmm3
        movdqa  10528(%rsi,%rcx),%xmm1
        movdqa  10528(%rsi,%r10),%xmm0
        pxor    %xmm2,%xmm1
        pxor    %xmm0,%xmm3
        pxor    %xmm1,%xmm0
        pxor    %xmm3,%xmm2
        por     %xmm3,%xmm0
        por     %xmm2,%xmm1
        movdqa  %xmm0,32(%rdx)
        movdqa  %xmm1,96(%rdx)

	movdqa	48(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	movdqa	112(%rdx),%xmm1
        movdqa  48(%rsi,%rax),%xmm2
        movdqa  48(%rsi,%rbx),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  5296(%rsi,%rbp),%xmm0
        movdqa  5296(%rsi,%r9),%xmm1
        pxor    %xmm3,%xmm0
        pxor    %xmm1,%xmm2
        pxor    %xmm0,%xmm1
        pxor    %xmm2,%xmm3
        por     %xmm1,%xmm2
        por     %xmm0,%xmm3
        movdqa  10544(%rsi,%rcx),%xmm1
        movdqa  10544(%rsi,%r10),%xmm0
        pxor    %xmm2,%xmm1
        pxor    %xmm0,%xmm3
        pxor    %xmm1,%xmm0
        pxor    %xmm3,%xmm2
        por     %xmm3,%xmm0
        por     %xmm2,%xmm1
        movdqa  %xmm0,48(%rdx)
        movdqa  %xmm1,112(%rdx)

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
        jne     pc3bma2
pc3bma5:
        popq    %rbp
        popq    %rbx
        ret      

/* input  rdi Afmt     rsi bwa     rdx  Cfmt  */

/* six registers point in bwa  0   rax   rbx  */
/*                             1   rbp   r9   */
/*                             2   rcx   r10  */
	.text
	.globl	pc3bmj
pc3bmj:
        pushq   %rbx
        pushq   %rbp
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pc3bmj5        /* yes - return           */
pc3bmj3:
	salq	$7, %rbx      /* 128 * byte Afmt        */
        addq    %rbx,%rdx     /* skip some rows of Cfmt */

        movq    %rcx,%rax    /*  adslice 0  */
        movq    %rcx,%rbp    /*  adslice 1  */
/*      movq    %rcx,%rcx    /*  adslice 2  */
        sarq    $2,%rax      /*  shift slice 0*/
        sarq    $10,%rbp     /*  shift slice 1*/
        sarq    $18,%rcx     /*  shift slice 2*/
        andq    $8128,%rax   /*  and slice 0*/
        andq    $8128,%rbp   /*  and slice 1  */   
        andq    $8128,%rcx   /*  and slice 2*/
        movq    %rax,%rbx    /*  adslice 0+ */
        movq    %rbp,%r9     /*  adslice 1+ */
        movq    %rcx,%r10    /*  adslice 2+ */
        xorq    $64,%rbx
        xorq    $64,%r9
        xorq    $64,%r10
        addq    $4,%rdi       /* point to next Afmt word*/

	vmovdqa	(%rdx),%ymm0   /* get 64 bytes of Cfmt */
	vmovdqa	64(%rdx),%ymm1
        vmovdqa  (%rsi,%rax),%ymm2
        vmovdqa  (%rsi,%rbx),%ymm3
        vpxor    %ymm0,%ymm2,%ymm2
        vpxor    %ymm3,%ymm1,%ymm1
        vpxor    %ymm2,%ymm3,%ymm3
        vpxor    %ymm1,%ymm0,%ymm0
        vpor     %ymm1,%ymm3,%ymm3
        vpor     %ymm0,%ymm2,%ymm2
        vmovdqa  5248(%rsi,%rbp),%ymm0
        vmovdqa  5248(%rsi,%r9),%ymm1
        vpxor    %ymm3,%ymm0,%ymm0
        vpxor    %ymm1,%ymm2,%ymm2
        vpxor    %ymm0,%ymm1,%ymm1
        vpxor    %ymm2,%ymm3,%ymm3
        vpor     %ymm1,%ymm2,%ymm2
        vpor     %ymm0,%ymm3,%ymm3
        vmovdqa  10496(%rsi,%rcx),%ymm1
        vmovdqa  10496(%rsi,%r10),%ymm0
        vpxor    %ymm2,%ymm1,%ymm1
        vpxor    %ymm0,%ymm3,%ymm3
        vpxor    %ymm1,%ymm0,%ymm0
        vpxor    %ymm3,%ymm2,%ymm2
        vpor     %ymm3,%ymm0,%ymm0
        vpor     %ymm2,%ymm1,%ymm1
        vmovdqa  %ymm0,(%rdx)
        vmovdqa  %ymm1,64(%rdx)

	vmovdqa	32(%rdx),%ymm0   /* get 64 bytes of Cfmt */
	vmovdqa	96(%rdx),%ymm1
        vmovdqa 32(%rsi,%rax),%ymm2
        vmovdqa 32(%rsi,%rbx),%ymm3
        vpxor    %ymm0,%ymm2,%ymm2
        vpxor    %ymm3,%ymm1,%ymm1
        vpxor    %ymm2,%ymm3,%ymm3
        vpxor    %ymm1,%ymm0,%ymm0
        vpor     %ymm1,%ymm3,%ymm3
        vpor     %ymm0,%ymm2,%ymm2
        vmovdqa  5280(%rsi,%rbp),%ymm0
        vmovdqa  5280(%rsi,%r9),%ymm1
        vpxor    %ymm3,%ymm0,%ymm0
        vpxor    %ymm1,%ymm2,%ymm2
        vpxor    %ymm0,%ymm1,%ymm1
        vpxor    %ymm2,%ymm3,%ymm3
        vpor     %ymm1,%ymm2,%ymm2
        vpor     %ymm0,%ymm3,%ymm3
        vmovdqa  10528(%rsi,%rcx),%ymm1
        vmovdqa  10528(%rsi,%r10),%ymm0
        vpxor    %ymm2,%ymm1,%ymm1
        vpxor    %ymm0,%ymm3,%ymm3
        vpxor    %ymm1,%ymm0,%ymm0
        vpxor    %ymm3,%ymm2,%ymm2
        vpor     %ymm3,%ymm0,%ymm0
        vpor     %ymm2,%ymm1,%ymm1
        vmovdqa  %ymm0,32(%rdx)
        vmovdqa  %ymm1,96(%rdx)

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
        jne     pc3bmj3
pc3bmj5:
        popq    %rbp
        popq    %rbx
        ret      


/* input  rdi Afmt     rsi bwa     rdx  Cfmt  */

/* six registers point in bwa  0   rax   rbx  */
/*                             1   rbp   r9   */
/*                             2   rcx   r10  */
	.text
	.globl	pc3bmm
pc3bmm:
        pushq   %rbx
        pushq   %rbp
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pc3bmm5        /* yes - return           */
pc3bmm3:
	salq	$7, %rbx      /* 128 * byte Afmt        */
        addq    %rbx,%rdx     /* skip some rows of Cfmt */

        movq    %rcx,%rax    /*  adslice 0  */
        movq    %rcx,%rbp    /*  adslice 1  */
/*      movq    %rcx,%rcx    /*  adslice 2  */
        sarq    $2,%rax      /*  shift slice 0*/
        sarq    $10,%rbp     /*  shift slice 1*/
        sarq    $18,%rcx     /*  shift slice 2*/
        andq    $8128,%rax   /*  and slice 0*/
        andq    $8128,%rbp   /*  and slice 1  */   
        andq    $8128,%rcx   /*  and slice 2*/
        movq    %rax,%rbx    /*  adslice 0+ */
        movq    %rbp,%r9     /*  adslice 1+ */
        movq    %rcx,%r10    /*  adslice 2+ */
        xorq    $64,%rbx
        xorq    $64,%r9
        xorq    $64,%r10
        addq    $4,%rdi       /* point to next Afmt word*/

	vmovdqa64  (%rdx),%zmm0   /* get 64 bytes of Cfmt */
	vmovdqa64  64(%rdx),%zmm1
        vmovdqa64  (%rsi,%rax),%zmm2
        vmovdqa64  (%rsi,%rbx),%zmm3
        vpxord    %zmm0,%zmm2,%zmm2
        vpxord    %zmm3,%zmm1,%zmm1
        vpternlogq $0xBE,%zmm1,%zmm2,%zmm3
        vpternlogq $0xF6,%zmm0,%zmm1,%zmm2
        vmovdqa64  5248(%rsi,%rbp),%zmm0
        vmovdqa64  5248(%rsi,%r9),%zmm1
        vpxord    %zmm3,%zmm0,%zmm0
        vpxord    %zmm1,%zmm2,%zmm2
        vpternlogq $0xBE,%zmm0,%zmm2,%zmm3
        vpternlogq $0xF6,%zmm0,%zmm1,%zmm2
        vmovdqa64  10496(%rsi,%rcx),%zmm1
        vmovdqa64  10496(%rsi,%r10),%zmm0
        vpxord    %zmm2,%zmm1,%zmm1
        vpxord    %zmm0,%zmm3,%zmm3
        vpternlogq $0xBE,%zmm3,%zmm1,%zmm0
        vpternlogq $0xF6,%zmm2,%zmm3,%zmm1
        vmovdqa64  %zmm0,(%rdx)
        vmovdqa64  %zmm1,64(%rdx)

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
        jne     pc3bmm3
pc3bmm5:
        popq    %rbp
        popq    %rbx
        ret  
