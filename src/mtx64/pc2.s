/* pcrits.s x86 assembler before AVX-512 changes*/
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

// pc2aca  rdi=prog rsi=bwa rdx stride
// %rax program byte
// r8 running program counter, %r9 destination r10 slices left

.text
	.globl	pc2aca
pc2aca: 
        movq        $8,%r10
pc2aca1:                        /* next slice */
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
pc2aca2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to second program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pc2aca4       /* no - go see what it is */
pc2aca3:
        shlq    $7,%rax       /* convert to displacement */
        pxor    0(%rax,%rsi),%xmm0    /* add in cauldron */
        pxor   16(%rax,%rsi),%xmm1
        pxor   32(%rax,%rsi),%xmm2
        pxor   48(%rax,%rsi),%xmm3
        pxor   64(%rax,%rsi),%xmm4
        pxor   80(%rax,%rsi),%xmm5
        pxor   96(%rax,%rsi),%xmm6
        pxor  112(%rax,%rsi),%xmm7
        movdqa  %xmm0,0(%r9)           /* store result */
        movdqa  %xmm1,16(%r9)          /* in destination */
        movdqa  %xmm2,32(%r9)
        movdqa  %xmm3,48(%r9)
        movdqa  %xmm4,64(%r9)
        movdqa  %xmm5,80(%r9)
        movdqa  %xmm6,96(%r9)
        movdqa  %xmm7,112(%r9)

        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pc2aca3      /* yes - go straight round again */
pc2aca4:
        cmpq    $159,%rax
        ja      pc2aca5      /* not a load of accumulator either */
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
        jmp     pc2aca2
pc2aca5:
        cmpq    $239,%rax
        ja      pc2aca6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pc2aca2

pc2aca6:                      /* anything 240+ is stop at the moment */
        addq    %rdx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pc2aca1
        ret

// end of pc2aca


// pc2acj  rdi=prog rsi=bwa (updated)
// %rax program byte
// r8 running program counter, %r9 destination r10 slices left

.text
	.globl	pc2acj
pc2acj: 
        movq        $8,%r10
pc2acj1:                        /* next slice */
        movq        %rsi,%r9
        addq        $256,%r9   /* destination starts slot 2! */
        movq        %rdi,%r8   /* restart program from beginning */
        vmovdqa -128(%r9),%ymm0  /* load accumulator from slot 1 */
        vmovdqa -96(%r9),%ymm1
        vmovdqa -64(%r9),%ymm2
        vmovdqa -32(%r9),%ymm3
pc2acj2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to second program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pc2acj4       /* no - go see what it is */
pc2acj3:
        shlq    $7,%rax       /* convert to displacement */
        vpxor    0(%rax,%rsi),%ymm0,%ymm0   /* add in cauldron */
        vpxor   32(%rax,%rsi),%ymm1,%ymm1
        vpxor   64(%rax,%rsi),%ymm2,%ymm2
        vpxor   96(%rax,%rsi),%ymm3,%ymm3
        vmovdqa  %ymm0,0(%r9)           /* store result */
        vmovdqa  %ymm1,32(%r9)          /* in destination */
        vmovdqa  %ymm2,64(%r9)
        vmovdqa  %ymm3,96(%r9)

        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pc2acj3      /* yes - go straight round again */
pc2acj4:
        cmpq    $159,%rax
        ja      pc2acj5      /* not a load of accumulator either */
        subq    $80,%rax
        shlq    $7,%rax        /* multiply by slot size */
        vmovdqa  0(%rax,%rsi),%ymm0
        vmovdqa  32(%rax,%rsi),%ymm1
        vmovdqa  64(%rax,%rsi),%ymm2
        vmovdqa  96(%rax,%rsi),%ymm3
        jmp     pc2acj2
pc2acj5:
        cmpq    $239,%rax
        ja      pc2acj6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pc2acj2

pc2acj6:                      /* anything 240+ is stop at the moment */
        addq    %rdx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pc2acj1
        ret

// end of pc2acj


// pc2acm  rdi=prog rsi=bwa (updated)
// %rax program byte
// r8 running program counter, %r9 destination r10 slices left

.text
	.globl	pc2acm
pc2acm: 
        movq        $8,%r10
pc2acm1:                        /* next slice */
        movq        %rsi,%r9
        addq        $256,%r9   /* destination starts slot 2! */
        movq        %rdi,%r8   /* restart program from beginning */
        vmovdqa64 -128(%r9),%zmm0  /* load accumulator from slot 1 */
        vmovdqa64 -64(%r9),%zmm1
pc2acm2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to second program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pc2acm4       /* no - go see what it is */
pc2acm3:
        shlq    $7,%rax       /* convert to displacement */
        vpxorq    0(%rax,%rsi),%zmm0,%zmm0   /* add in cauldron */
        vpxorq   64(%rax,%rsi),%zmm1,%zmm1
        vmovdqa64  %zmm0,0(%r9)           /* store result */
        vmovdqa64  %zmm1,64(%r9)

        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pc2acm3      /* yes - go straight round again */
pc2acm4:
        cmpq    $159,%rax
        ja      pc2acm5      /* not a load of accumulator either */
        subq    $80,%rax
        shlq    $7,%rax        /* multiply by slot size */
        vmovdqa64  0(%rax,%rsi),%zmm0
        vmovdqa64  64(%rax,%rsi),%zmm1
        jmp     pc2acm2
pc2acm5:
        cmpq    $239,%rax
        ja      pc2acm6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pc2acm2

pc2acm6:                      /* anything 240+ is stop at the moment */
        addq    %rdx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pc2acm1
        ret

// end of pc2acm

/* rdi Afmt     rsi bwa     rdx  Cfmt  */
/* rcx count  rax/r8 bwa addresses r9 bwa ptr*/
	.text
	.globl	pc2bma
pc2bma:
        pushq   %rbx
        pushq   %rbp

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pc2bma5        /* yes - return           */
pc2bma2:
	salq	$7, %rbx      /* 128 * byte Afmt        */
        addq    %rbx,%rdx     /* skip some rows of Cfmt */

        movq    %rcx,%rax    /*  adslice 0*/
        movq    %rcx,%rbp    /*  adslice 1    */
        movq    %rcx,%r8     /*  adslice 2*/
        movq    %rcx,%rbx    /*  adslice 3*/
        movq    %rcx,%r9     /*  adslice 4*/
        movq    %rcx,%r10    /*  adslice 5*/
        movq    %rcx,%r11    /*  adslice 6*/
/*      movq    %rcx,%rcx        adslice 7*/

        sarq    $1,%rax      /*  shift slice 0*/
        sarq    $5,%rbp      /*  shift slice 1*/     
        sarq    $9,%r8       /*  shift slice 2*/
        sarq    $13,%rbx     /*  shift slice 3*/
        sarq    $17,%r9      /*  shift slice 4*/
        sarq    $21,%r10     /*  shift slice 5*/
        sarq    $25,%r11     /*  shift slice 6*/
        sarq    $29,%rcx     /*  shift slice 7*/

        andq    $1920,%rax   /*  and slice 0*/
        andq    $1920,%rbp   /*  and slice 1  */   
        andq    $1920,%r8    /*  and slice 2*/
        andq    $1920,%rbx   /*  and slice 3*/
        andq    $1920,%r9    /*  and slice 4*/
        andq    $1920,%r10   /*  and slice 5*/
        andq    $1920,%r11   /*  and slice 6*/
        andq    $1920,%rcx   /*  and slice 7*/

        addq    $5,%rdi       /* point to next Afmt word*/

	movdqa	0(%rdx), %xmm0 /* get 128 bytes of Cfmt */
	movdqa	16(%rdx), %xmm1
	movdqa	32(%rdx), %xmm2
	movdqa	48(%rdx), %xmm3
	movdqa	64(%rdx), %xmm4
	movdqa	80(%rdx), %xmm5
	movdqa	96(%rdx), %xmm6
	movdqa	112(%rdx), %xmm7

        pxor    0(%rsi,%rax),%xmm0
        pxor    16(%rsi,%rax),%xmm1
        pxor    32(%rsi,%rax),%xmm2
        pxor    48(%rsi,%rax),%xmm3
        pxor    64(%rsi,%rax),%xmm4
        pxor    80(%rsi,%rax),%xmm5
        pxor    96(%rsi,%rax),%xmm6
        pxor    112(%rsi,%rax),%xmm7

        pxor    2048(%rsi,%rbp),%xmm0
        pxor    2064(%rsi,%rbp),%xmm1
        pxor    2080(%rsi,%rbp),%xmm2
        pxor    2096(%rsi,%rbp),%xmm3
        pxor    2112(%rsi,%rbp),%xmm4
        pxor    2128(%rsi,%rbp),%xmm5
        pxor    2144(%rsi,%rbp),%xmm6
        pxor    2160(%rsi,%rbp),%xmm7

        pxor    4096(%rsi,%r8),%xmm0
        pxor    4112(%rsi,%r8),%xmm1
        pxor    4128(%rsi,%r8),%xmm2
        pxor    4144(%rsi,%r8),%xmm3
        pxor    4160(%rsi,%r8),%xmm4
        pxor    4176(%rsi,%r8),%xmm5
        pxor    4192(%rsi,%r8),%xmm6
        pxor    4208(%rsi,%r8),%xmm7

        pxor    6144(%rsi,%rbx),%xmm0
        pxor    6160(%rsi,%rbx),%xmm1
        pxor    6176(%rsi,%rbx),%xmm2
        pxor    6192(%rsi,%rbx),%xmm3
        pxor    6208(%rsi,%rbx),%xmm4
        pxor    6224(%rsi,%rbx),%xmm5
        pxor    6240(%rsi,%rbx),%xmm6
        pxor    6256(%rsi,%rbx),%xmm7

        pxor    8192(%rsi,%r9),%xmm0
        pxor    8208(%rsi,%r9),%xmm1
        pxor    8224(%rsi,%r9),%xmm2
        pxor    8240(%rsi,%r9),%xmm3
        pxor    8256(%rsi,%r9),%xmm4
        pxor    8272(%rsi,%r9),%xmm5
        pxor    8288(%rsi,%r9),%xmm6
        pxor    8304(%rsi,%r9),%xmm7

        pxor    10240(%rsi,%r10),%xmm0
        pxor    10256(%rsi,%r10),%xmm1
        pxor    10272(%rsi,%r10),%xmm2
        pxor    10288(%rsi,%r10),%xmm3
        pxor    10304(%rsi,%r10),%xmm4
        pxor    10320(%rsi,%r10),%xmm5
        pxor    10336(%rsi,%r10),%xmm6
        pxor    10352(%rsi,%r10),%xmm7

        pxor    12288(%rsi,%r11),%xmm0
        pxor    12304(%rsi,%r11),%xmm1
        pxor    12320(%rsi,%r11),%xmm2
        pxor    12336(%rsi,%r11),%xmm3
        pxor    12352(%rsi,%r11),%xmm4
        pxor    12368(%rsi,%r11),%xmm5
        pxor    12384(%rsi,%r11),%xmm6
        pxor    12400(%rsi,%r11),%xmm7

        pxor    14336(%rsi,%rcx),%xmm0
        pxor    14352(%rsi,%rcx),%xmm1
        pxor    14368(%rsi,%rcx),%xmm2
        pxor    14384(%rsi,%rcx),%xmm3
        pxor    14400(%rsi,%rcx),%xmm4
        pxor    14416(%rsi,%rcx),%xmm5
        pxor    14432(%rsi,%rcx),%xmm6
        pxor    14448(%rsi,%rcx),%xmm7

        movdqa  %xmm0,0(%rdx)
        movdqa  %xmm1,16(%rdx)
        movdqa  %xmm2,32(%rdx)
        movdqa  %xmm3,48(%rdx)
        movdqa  %xmm4,64(%rdx)
        movdqa  %xmm5,80(%rdx)
        movdqa  %xmm6,96(%rdx)
        movdqa  %xmm7,112(%rdx)

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
        jne     pc2bma2
pc2bma5:
        popq    %rbp
        popq    %rbx
  
        ret      

/* rdi Afmt     rsi bwa     rdx  Cfmt  */
/* rcx count  rax/r8 bwa addresses r9 bwa ptr*/
	.text
	.globl	pc2bmj
pc2bmj:
        pushq   %rbx
        pushq   %rbp

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pc2bmj5        /* yes - return           */
pc2bmj1:
	salq	$7, %rbx      /* 128 * byte Afmt        */
        addq    %rbx,%rdx     /* skip some rows of Cfmt */
	vmovdqa	0(%rdx), %ymm0 /* get 128 bytes of Cfmt */

	vmovdqa	64(%rdx), %ymm4

        movq    %rcx,%rax    /*  adslice 0*/
        sarq    $29,%rcx     /*  shift slice 7*/
        andq    $1920,%rcx   /*  and slice 7*/
	vmovdqa	32(%rdx), %ymm2
	vmovdqa	96(%rdx), %ymm6
        vpxor    14336(%rsi,%rcx),%ymm0,%ymm0
        vpxor    14400(%rsi,%rcx),%ymm4,%ymm4
        movq    %rax,%rbp    /*  adslice 1    */
        sarq    $1,%rax      /*  shift slice 0*/
        andq    $1920,%rax   /*  and slice 0*/
        vpxor    14368(%rsi,%rcx),%ymm2,%ymm2
        vpxor    14432(%rsi,%rcx),%ymm6,%ymm6
        vpxor    0(%rsi,%rax),%ymm0,%ymm0
        vpxor    64(%rsi,%rax),%ymm4,%ymm4
        movq    %rbp,%r8     /*  adslice 2*/
        sarq    $5,%rbp      /*  shift slice 1*/
        andq    $1920,%rbp   /*  and slice 1  */
        vpxor    32(%rsi,%rax),%ymm2,%ymm2
        vpxor    96(%rsi,%rax),%ymm6,%ymm6
        vpxor    2048(%rsi,%rbp),%ymm0,%ymm0
        vpxor    2112(%rsi,%rbp),%ymm4,%ymm4
        movq    %r8,%rbx    /*  adslice 3*/
        sarq    $9,%r8       /*  shift slice 2*/
        andq    $1920,%r8    /*  and slice 2*/
        vpxor    2080(%rsi,%rbp),%ymm2,%ymm2
        vpxor    2144(%rsi,%rbp),%ymm6,%ymm6
        vpxor    4096(%rsi,%r8),%ymm0,%ymm0
        vpxor    4160(%rsi,%r8),%ymm4,%ymm4
        movq    %rbx,%r9     /*  adslice 4*/
        sarq    $13,%rbx     /*  shift slice 3*/
        andq    $1920,%rbx   /*  and slice 3*/
        vpxor    4128(%rsi,%r8),%ymm2,%ymm2
        vpxor    4192(%rsi,%r8),%ymm6,%ymm6
        vpxor    6144(%rsi,%rbx),%ymm0,%ymm0
        vpxor    6208(%rsi,%rbx),%ymm4,%ymm4
        movq    %r9,%r10    /*  adslice 5*/
        sarq    $17,%r9      /*  shift slice 4*/
        andq    $1920,%r9    /*  and slice 4*/
        vpxor    6176(%rsi,%rbx),%ymm2,%ymm2
        vpxor    6240(%rsi,%rbx),%ymm6,%ymm6
        vpxor    8192(%rsi,%r9),%ymm0,%ymm0
        vpxor    8256(%rsi,%r9),%ymm4,%ymm4
        movq    %r10,%r11    /*  adslice 6*/
        sarq    $21,%r10     /*  shift slice 5*/
        andq    $1920,%r10   /*  and slice 5*/
        vpxor    8224(%rsi,%r9),%ymm2,%ymm2
        vpxor    8288(%rsi,%r9),%ymm6,%ymm6
        vpxor    10240(%rsi,%r10),%ymm0,%ymm0
        vpxor    10304(%rsi,%r10),%ymm4,%ymm4
        sarq    $25,%r11     /*  shift slice 6*/
        andq    $1920,%r11   /*  and slice 6*/
        vpxor    10272(%rsi,%r10),%ymm2,%ymm2
        vpxor    10336(%rsi,%r10),%ymm6,%ymm6
        vpxor    12288(%rsi,%r11),%ymm0,%ymm0
        vpxor    12352(%rsi,%r11),%ymm4,%ymm4
        vpxor    12320(%rsi,%r11),%ymm2,%ymm2
        vpxor    12384(%rsi,%r11),%ymm6,%ymm6

        addq    $5,%rdi       /* point to next Afmt word*/

        vmovdqa  %ymm0,0(%rdx)
        vmovdqa  %ymm4,64(%rdx)
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        vmovdqa  %ymm2,32(%rdx)
        vmovdqa  %ymm6,96(%rdx)

        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
        jne     pc2bmj1
pc2bmj5:
        popq    %rbp
        popq    %rbx  
        ret      


/* rdi Afmt     rsi bwa     rdx  Cfmt  */
/* rcx count  rax/r8 bwa addresses r9 bwa ptr*/
	.text
	.globl	pc2bmm
pc2bmm:
        pushq   %rbx
        pushq   %rbp

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pc2bmm5        /* yes - return           */
pc2bmm1:
	salq	$7, %rbx      /* 128 * byte Afmt        */
        addq    %rbx,%rdx     /* skip some rows of Cfmt */
	vmovdqa64  0(%rdx), %zmm0 /* get 128 bytes of Cfmt */
	vmovdqa64  64(%rdx), %zmm4

        movq    %rcx,%rax    /*  adslice 0*/
        sarq    $29,%rcx     /*  shift slice 7*/
        andq    $1920,%rcx   /*  and slice 7*/
        vmovdqa64 14336(%rsi,%rcx),%zmm1
        vmovdqa64 14400(%rsi,%rcx),%zmm5
        movq    %rax,%rbp    /*  adslice 1    */
        sarq    $1,%rax      /*  shift slice 0*/
        andq    $1920,%rax   /*  and slice 0*/
        vpternlogq $0x96,0(%rsi,%rax),%zmm1,%zmm0
        vpternlogq $0x96,64(%rsi,%rax),%zmm5,%zmm4
        movq    %rbp,%r8     /*  adslice 2*/
        sarq    $5,%rbp      /*  shift slice 1*/
        andq    $1920,%rbp   /*  and slice 1  */
        vmovdqa64 2048(%rsi,%rbp),%zmm1
        vmovdqa64 2112(%rsi,%rbp),%zmm5
        movq    %r8,%rbx    /*  adslice 3*/
        sarq    $9,%r8       /*  shift slice 2*/
        andq    $1920,%r8    /*  and slice 2*/
        vpternlogq $0x96,4096(%rsi,%r8),%zmm1,%zmm0
        vpternlogq $0x96,4160(%rsi,%r8),%zmm5,%zmm4
        movq    %rbx,%r9     /*  adslice 4*/
        sarq    $13,%rbx     /*  shift slice 3*/
        andq    $1920,%rbx   /*  and slice 3*/
        vmovdqa64 6144(%rsi,%rbx),%zmm1
        vmovdqa64 6208(%rsi,%rbx),%zmm5
        movq    %r9,%r10    /*  adslice 5*/
        sarq    $17,%r9      /*  shift slice 4*/
        andq    $1920,%r9    /*  and slice 4*/
        vpternlogq $0x96,8192(%rsi,%r9),%zmm1,%zmm0
        vpternlogq $0x96,8256(%rsi,%r9),%zmm5,%zmm4
        movq    %r10,%r11    /*  adslice 6*/
        sarq    $21,%r10     /*  shift slice 5*/
        andq    $1920,%r10   /*  and slice 5*/
        vmovdqa64 10240(%rsi,%r10),%zmm1
        vmovdqa64 10304(%rsi,%r10),%zmm5
        sarq    $25,%r11     /*  shift slice 6*/
        andq    $1920,%r11   /*  and slice 6*/
        vpternlogq $0x96,12288(%rsi,%r11),%zmm1,%zmm0
        vpternlogq $0x96,12352(%rsi,%r11),%zmm5,%zmm4

        addq    $5,%rdi       /* point to next Afmt word*/

        vmovdqa64  %zmm0,0(%rdx)
        vmovdqa64  %zmm4,64(%rdx)
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */

        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
        jne     pc2bmm1
pc2bmm5:
        popq    %rbp
        popq    %rbx  
        ret      
