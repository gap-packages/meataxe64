/* pcrit2s.s assembler for AVX-2 version */
/* R. A. Parker 27.9.2016  */
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

// void pcbunf(Dfmt * d, const Dfmt * s, uint64_t nob,
//               %rdi            %rsi         %rdx
//                   const uint8_t * t1, const uint8_t * t2)
//                         %rcx              %r8
	.text
	.globl	pcbunf
	.type	pcbunf, @function
pcbunf:

        movq    %rdx,%r11      /* count out of ancient regs */
        movq    %rsi,%r10
        movq    %r8,%rsi
        cmpq    $0,%r11
        je      pcbunf2
        cmpq    $9,%r11
        jc      pcbunf3
        subq    $9,%r11

pcbunf1:
        movzbq  (%r10),%rdx    
        movzbq  (%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,(%rdi)

        movzbq  1(%r10),%rdx    
        movzbq  1(%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,1(%rdi)

        movzbq  2(%r10),%rdx    
        movzbq  2(%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,2(%rdi)

        movzbq  3(%r10),%rdx    
        movzbq  3(%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,3(%rdi)

        movzbq  4(%r10),%rdx    
        movzbq  4(%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,4(%rdi)

        movzbq  5(%r10),%rdx    
        movzbq  5(%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,5(%rdi)

        movzbq  6(%r10),%rdx    
        movzbq  6(%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,6(%rdi)

        movzbq  7(%r10),%rdx    
        movzbq  7(%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,7(%rdi)

        addq    $8,%rdi
        addq    $8,%r10
        subq    $8,%r11
        jnc     pcbunf1

        addq    $9,%r11

pcbunf3:
        movzbq  (%r10),%rdx    
        movzbq  (%rdi),%rax 
        movzbq  (%rcx,%rdx),%r9  
        shlq    $8,%r9
        addq    %r9,%rax
        movb    (%rsi,%rax),%ah   
        movb    %ah,(%rdi)      
        addq    $1,%rdi
        addq    $1,%r10
        subq    $1,%r11
        jne     pcbunf3
pcbunf2:
        ret
	.size	pcbunf, .-pcbunf

// void pcxunf(Dfmt * d, const Dfmt * s, uint64_t nob,
//               %rdi            %rsi         %rdx
//                   const uint8_t * t1)
//                         %rcx
	.text
	.globl	pcxunf
	.type	pcxunf, @function
pcxunf:
        cmpq    $8,%rdx
        jb      pcxunf6
pcxunf1:
        movzbq  (%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,(%rdi)
        movzbq  1(%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,1(%rdi)
        movzbq  2(%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,2(%rdi)
        movzbq  3(%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,3(%rdi)
        movzbq  4(%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,4(%rdi)
        movzbq  5(%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,5(%rdi)
        movzbq  6(%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,6(%rdi)
        movzbq  7(%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,7(%rdi)
        addq    $8,%rdi
        addq    $8,%rsi
        subq    $8,%rdx
        cmpq    $8,%rdx
        jae     pcxunf1

pcxunf6:
        cmpq    $0,%rdx
        je      pcxunf8
pcxunf7:
        movzbq  (%rsi),%rax    /* get unary input */
        movb    (%rcx,%rax),%ah
        xorb    %ah,(%rdi)
        addq    $1,%rdi
        addq    $1,%rsi
        subq    $1,%rdx
        jne     pcxunf7
pcxunf8:
        ret
	.size	pcxunf, .-pcxunf


// void pcunf(Dfmt * d, uint64_t nob, const uint8_t * t1)
//              %rdi          %rsi         %rdx

	.text
	.globl	pcunf
	.type	pcunf, @function
pcunf:
        cmpq    $0,%rsi
        je      pcunf2
pcunf1:
        movzbq  (%rdi),%rax    /* get unary input */
        movb    (%rdx,%rax),%ch
        movb    %ch,(%rdi)
        addq    $1,%rdi
        subq    $1,%rsi
        jne     pcunf1
pcunf2:
        ret
	.size	pcunf, .-pcunf


// uint64_t pcpmad(uint64_t p,uint64_t a,uint64_t b,uint64_t c);
//  %rax             %rdi         %rsi       %rdx       %rcx
	.text
	.globl	pcpmad
	.type	pcpmad, @function
pcpmad:
        movq    %rdx,%rax
        mulq    %rsi
        addq    %rcx,%rax
        adcq    $0,%rdx
        divq    %rdi      /* %rax quot,  %rdx rem  */
        movq    %rdx,%rax
        ret      
	.size	pcpmad, .-pcpmad

//  pcxor(d.s1,s2,nob)  d = s1^s2 (all nob bytes long)
//                     rdi rsi rdx     rcx

	.text
	.globl	pcxor
	.type	pcxor, @function
pcxor:
        xorq    %rax,%rax
        cmpq    $32,%rcx
        jb      pcxor5
        subq    $32,%rcx

pcxor1:
        vmovdqu  (%rsi,%rax),%ymm0
        vmovdqu  (%rdx,%rax),%ymm1
        vpxor    %ymm1,%ymm0,%ymm0
        vmovdqu  %ymm0,(%rdi,%rax)
        addq    $32,%rax
        cmpq    %rcx,%rax
        jbe     pcxor1

        addq    $32,%rcx
pcxor5:
        cmpq    %rcx,%rax
        je      pcxor9
pcxor7:
        movb    (%rsi,%rax),%r8b
        xorb    (%rdx,%rax),%r8b
        movb    %r8b,(%rdi,%rax)
        addq    $1,%rax
        cmpq    %rcx,%rax
        jb      pcxor7

pcxor9:
        ret      
	.size	pcxor, .-pcxor


//  pcbif(d,s1,s2,nob,t)  a = t[(s1*256)+s2] (all nob bytes long)
//                        di  r8[(si*256)+dx]   cx
	.text
	.globl	pcbif
	.type	pcbif, @function
pcbif:
        pushq   %rbx
        pushq   %rbp
        pushq   %r15
        movq    %rcx,%r15
        movq    %rdx,%r9
        xchgq   %r8,%rsi
        cmpq    $0,%r15
        je      pcbif9
        xorq    %rbp,%rbp
        cmpq    $8,%r15
        jbe     pcbif8

        subq    $4,%r15

        movzwq  (%r8,%rbp),%rbx
        movzwq  (%r9,%rbp),%rax
        xchgb   %ah,%bl
        movb    (%rsi,%rax),%ch
        movb    (%rsi,%rbx),%dh
        movzwq  2(%r8,%rbp),%rbx
        movzwq  2(%r9,%rbp),%rax
        xchgb   %ah,%bl
        movb    %ch,(%rdi,%rbp)
        movb    (%rsi,%rax),%ch
        movb    %dh,1(%rdi,%rbp)
        movb    (%rsi,%rbx),%dh
        addq    $4,%rbp

pcbif4:

        movzwq  (%r8,%rbp),%rbx
        movzwq  (%r9,%rbp),%rax
        xchgb   %ah,%bl
        movb    %ch,-2(%rdi,%rbp)
        movb    (%rsi,%rax),%ch
        movb    %dh,-1(%rdi,%rbp)
        movb    (%rsi,%rbx),%dh

        movzwq  2(%r8,%rbp),%rbx
        movzwq  2(%r9,%rbp),%rax
        xchgb   %ah,%bl
        movb    %ch,(%rdi,%rbp)
        movb    (%rsi,%rax),%ch
        movb    %dh,1(%rdi,%rbp)
        movb    (%rsi,%rbx),%dh

        addq    $4,%rbp
        cmpq    %r15,%rbp
        jb      pcbif4

        movb    %ch,-2(%rdi,%rbp)
        movb    %dh,-1(%rdi,%rbp)

        addq    $4,%r15

pcbif8:
        movzbq  (%r8,%rbp),%r10
        movzbq  (%r9,%rbp),%r11
        shlq    $8,%r10
        addq    %r11,%r10
        movb    (%rsi,%r10),%r11b
        movb    %r11b,(%rdi,%rbp)
        addq    $1,%rbp
        cmpq    %r15,%rbp
        jb      pcbif8
pcbif9:
        popq    %r15
        popq    %rbp
        popq    %rbx
        ret

       .size	pcbif, .-pcbif

/* rdi Afmt     rsi bwa     rdx  Cfmt  */
/* rcx count  rax/r8 bwa addresses r9 bwa ptr*/
	.text
	.globl	pcbm2
	.type	pcbm2, @function
pcbm2:
        pushq   %rbx
        pushq   %rbp

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      getout        /* yes - return           */
loop1:
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
        jne     loop1
getout:
        popq    %rbp
        popq    %rbx
  
        ret      
	.size	pcbm2, .-pcbm2

/* pcad3 a, b, c.  c=a+b  */
/* rdi input1   rsi input2  rdx output  */

	.text
	.globl	pcad3
	.type	pcad3, @function
pcad3:
	vmovdqa	(%rdi),%ymm0
	vmovdqa	64(%rdi),%ymm1
        vmovdqa  (%rsi),%ymm2
        vmovdqa  64(%rsi),%ymm3
        vpxor    %ymm0,%ymm2,%ymm2
        vpxor    %ymm3,%ymm1,%ymm1
        vpxor    %ymm2,%ymm3,%ymm3
        vpxor    %ymm1,%ymm0,%ymm0
        vpor     %ymm1,%ymm3,%ymm3
        vpor     %ymm0,%ymm2,%ymm2
        vmovdqa  %ymm3,(%rdx)
        vmovdqa  %ymm2,64(%rdx)

	vmovdqa	32(%rdi),%ymm0
	vmovdqa	96(%rdi),%ymm1
        vmovdqa  32(%rsi),%ymm2
        vmovdqa  96(%rsi),%ymm3
        vpxor    %ymm0,%ymm2,%ymm2
        vpxor    %ymm3,%ymm1,%ymm1
        vpxor    %ymm2,%ymm3,%ymm3
        vpxor    %ymm1,%ymm0,%ymm0
        vpor     %ymm1,%ymm3,%ymm3
        vpor     %ymm0,%ymm2,%ymm2
        vmovdqa  %ymm3,32(%rdx)
        vmovdqa  %ymm2,96(%rdx)

        ret      
	.size	pcad3, .-pcad3


/* input  rdi Afmt     rsi bwa     rdx  Cfmt  */

/* six registers point in bwa  0   rax   rbx  */
/*                             1   rbp   r9   */
/*                             2   rcx   r10  */
	.text
	.globl	pcbm3
	.type	pcbm3, @function
pcbm3:
        pushq   %rbx
        pushq   %rbp
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      getout3       /* yes - return           */
loop3:
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
        jne     loop3
getout3:
        popq    %rbp
        popq    %rbx
        ret      
	.size	pcbm3, .-pcbm3

/*  4AS module routines   */
/*========================*/

/* pc4as Brick Mad for 16-bit 4AS HPMI  */

/* pc4as (uint64_t Afmt, BKfmt bwa, Cfmt c, PHS p */
/*             %rdi        %rsi      %rdx    %rcx */

/* PHS  uint16_t prime                */
/* ===  uint16_t rem  = -8192 % p     */
/*      uint16_t bias = multiple of p */

/* REGISTER ALLOCATION  */

/* ymm8-ymm15 Cfmt    */
/* ymm7 rem           */
/* ymm6 bias          */
/* ymm5 mask          */

/* %rcx   reduce-flag - zero => don't */
/* %rdi   pointer to Afmt             */
/* %rsi   base of the brick           */
/* %rdx   pointer to the Cfmt         */
/* %rax   word of Afmt                */
/* %r8    address of brick slot       */

	.text
	.globl	pc4as
	.type	pc4as, @function
pc4as:
        testq   %rcx,%rcx
        je      pc4as1
        movq    $57344,%rax
        pushq   %rax
     vpbroadcastw  (%rsp),%ymm5    /* AVX-2 mask*/
        popq    %rax

     vpbroadcastw  2(%rcx),%ymm7   /* AVX-2 rem */
     vpbroadcastw  4(%rcx),%ymm6   /* AVX-2 bias*/

pc4as1:
        movq    (%rdi),%rax        /* get Afmt word */
        cmpb    $255,%al           /* Termination?  */
        je      pc4as9             /* yes - get out */

        movzbq  %al,%r8            /* extend skip to 64 bits */
        shlq    $8,%r8             /* multiply by 256 */
        addq    %r8,%rdx           /* and add to Cfmt pointer */
        
        vmovdqa  (%rdx),%ymm8       /* Load up the Cfmt */
        vmovdqa  32(%rdx),%ymm9
        vmovdqa  64(%rdx),%ymm10
        vmovdqa  96(%rdx),%ymm11
        vmovdqa  128(%rdx),%ymm12
        vmovdqa  160(%rdx),%ymm13
        vmovdqa  192(%rdx),%ymm14
        vmovdqa  224(%rdx),%ymm15

        movq    %rax,%r8
        shrq    $49,%r8
        andq    $32512,%r8
        addq    %rsi,%r8           /* or is SIB better?  */
        vpaddw   (%r8),%ymm8,%ymm8
        vpaddw   32(%r8),%ymm9,%ymm9
        vpaddw   64(%r8),%ymm10,%ymm10
        vpaddw   96(%r8),%ymm11,%ymm11
        vpaddw   128(%r8),%ymm12,%ymm12
        vpaddw   160(%r8),%ymm13,%ymm13
        vpaddw   192(%r8),%ymm14,%ymm14
        vpaddw   224(%r8),%ymm15,%ymm15
        movq    %rax,%r8
        shrq    $42,%r8
        andq    $32512,%r8
        addq    %rsi,%r8     
        vpsubw   (%r8),%ymm8,%ymm8
        vpsubw   32(%r8),%ymm9,%ymm9
        vpsubw   64(%r8),%ymm10,%ymm10
        vpsubw   96(%r8),%ymm11,%ymm11
        vpsubw   128(%r8),%ymm12,%ymm12
        vpsubw   160(%r8),%ymm13,%ymm13
        vpsubw   192(%r8),%ymm14,%ymm14
        vpsubw   224(%r8),%ymm15,%ymm15

        movq    %rax,%r8
        shrq    $35,%r8
        andq    $32512,%r8
        addq    %rsi,%r8  
        vpaddw   (%r8),%ymm8,%ymm8
        vpaddw   32(%r8),%ymm9,%ymm9
        vpaddw   64(%r8),%ymm10,%ymm10
        vpaddw   96(%r8),%ymm11,%ymm11
        vpaddw   128(%r8),%ymm12,%ymm12
        vpaddw   160(%r8),%ymm13,%ymm13
        vpaddw   192(%r8),%ymm14,%ymm14
        vpaddw   224(%r8),%ymm15,%ymm15
        movq    %rax,%r8
        shrq    $28,%r8
        andq    $32512,%r8
        addq    %rsi,%r8      
        vpsubw   (%r8),%ymm8,%ymm8
        vpsubw   32(%r8),%ymm9,%ymm9
        vpsubw   64(%r8),%ymm10,%ymm10
        vpsubw   96(%r8),%ymm11,%ymm11
        vpsubw   128(%r8),%ymm12,%ymm12
        vpsubw   160(%r8),%ymm13,%ymm13
        vpsubw   192(%r8),%ymm14,%ymm14
        vpsubw   224(%r8),%ymm15,%ymm15

        movq    %rax,%r8
        shrq    $21,%r8
        andq    $32512,%r8
        addq    %rsi,%r8     
        vpaddw   (%r8),%ymm8,%ymm8
        vpaddw   32(%r8),%ymm9,%ymm9
        vpaddw   64(%r8),%ymm10,%ymm10
        vpaddw   96(%r8),%ymm11,%ymm11
        vpaddw   128(%r8),%ymm12,%ymm12
        vpaddw   160(%r8),%ymm13,%ymm13
        vpaddw   192(%r8),%ymm14,%ymm14
        vpaddw   224(%r8),%ymm15,%ymm15
        movq    %rax,%r8
        shrq    $14,%r8
        andq    $32512,%r8
        addq    %rsi,%r8          
        vpsubw   (%r8),%ymm8,%ymm8
        vpsubw   32(%r8),%ymm9,%ymm9
        vpsubw   64(%r8),%ymm10,%ymm10
        vpsubw   96(%r8),%ymm11,%ymm11
        vpsubw   128(%r8),%ymm12,%ymm12
        vpsubw   160(%r8),%ymm13,%ymm13
        vpsubw   192(%r8),%ymm14,%ymm14
        vpsubw   224(%r8),%ymm15,%ymm15

        movq    %rax,%r8
        shrq    $7,%r8
        andq    $32512,%r8
        addq    %rsi,%r8
        vpaddw   (%r8),%ymm8,%ymm8
        vpaddw   32(%r8),%ymm9,%ymm9
        vpaddw   64(%r8),%ymm10,%ymm10
        vpaddw   96(%r8),%ymm11,%ymm11
        vpaddw   128(%r8),%ymm12,%ymm12
        vpaddw   160(%r8),%ymm13,%ymm13
        vpaddw   192(%r8),%ymm14,%ymm14
        vpaddw   224(%r8),%ymm15,%ymm15
        movq    %rax,%r8
//      shrq    $0,%r8
        andq    $32512,%r8
        addq    %rsi,%r8  
        vpsubw   (%r8),%ymm8,%ymm8
        vpsubw   32(%r8),%ymm9,%ymm9
        vpsubw   64(%r8),%ymm10,%ymm10
        vpsubw   96(%r8),%ymm11,%ymm11
        vpsubw   128(%r8),%ymm12,%ymm12
        vpsubw   160(%r8),%ymm13,%ymm13
        vpsubw   192(%r8),%ymm14,%ymm14
        vpsubw   224(%r8),%ymm15,%ymm15
        cmpq    $0,%rcx             /* do we need to reduce? */
        je      pc4as6

/* reduce, though not completely */

        vpand   %ymm5,%ymm8,%ymm4
        vpxor   %ymm4,%ymm8,%ymm8
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm8,%ymm8
        vpaddw  %ymm6,%ymm8,%ymm8

        vpand   %ymm5,%ymm9,%ymm4
        vpxor   %ymm4,%ymm9,%ymm9
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm9,%ymm9
        vpaddw  %ymm6,%ymm9,%ymm9

        vpand   %ymm5,%ymm10,%ymm4
        vpxor   %ymm4,%ymm10,%ymm10
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm10,%ymm10
        vpaddw  %ymm6,%ymm10,%ymm10

        vpand   %ymm5,%ymm11,%ymm4
        vpxor   %ymm4,%ymm11,%ymm11
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm11,%ymm11
        vpaddw  %ymm6,%ymm11,%ymm11

        vpand   %ymm5,%ymm12,%ymm4
        vpxor   %ymm4,%ymm12,%ymm12
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm12,%ymm12
        vpaddw  %ymm6,%ymm12,%ymm12

        vpand   %ymm5,%ymm13,%ymm4
        vpxor   %ymm4,%ymm13,%ymm13
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm13,%ymm13
        vpaddw  %ymm6,%ymm13,%ymm13

        vpand   %ymm5,%ymm14,%ymm4
        vpxor   %ymm4,%ymm14,%ymm14
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm14,%ymm14
        vpaddw  %ymm6,%ymm14,%ymm14

        vpand   %ymm5,%ymm15,%ymm4
        vpxor   %ymm4,%ymm15,%ymm15
        vpsllw  $13,%ymm4,%ymm4
        vpmullw %ymm7,%ymm4,%ymm4
        vpsubw  %ymm4,%ymm15,%ymm15
        vpaddw  %ymm6,%ymm15,%ymm15

pc4as6:
        vmovdqa %ymm8,(%rdx)        /* Put Cfmt back */
        vmovdqa %ymm9,32(%rdx)
        vmovdqa %ymm10,64(%rdx)
        vmovdqa %ymm11,96(%rdx)
        vmovdqa %ymm12,128(%rdx)
        vmovdqa %ymm13,160(%rdx)
        vmovdqa %ymm14,192(%rdx)
        vmovdqa %ymm15,224(%rdx)
        jmp     pc4as1

pc4as9:
        ret      
	.size	pc4as, .-pc4as


/* pc4asp Brick Populate 16-bit 4AS HPMI  */
/* pc4as (uint8_t cmds, BKfmt bwa,  PHS p */
/*             %rdi        %rsi      %rdx */

	.text
	.globl	pc4asp
	.type	pc4asp, @function
pc4asp:
        movzwq    (%rdx),%rax
        subq      $1,%rax
        pushq     %rax
     vpbroadcastw  (%rsp),%ymm1     /* p - 1 */
        popq      %rax
     vpbroadcastw (%rdx),%ymm0    /* prime */ 
    

pc4asp1:
        movzbq    (%rdi),%rax
        cmpq      $255,%rax
        je        pc4asp9
        shlq      $8,%rax
        movzbq    1(%rdi),%rcx
        shlq      $8,%rcx
        movzbq    2(%rdi),%rdx
        shlq      $8,%rdx

        vmovdqa   (%rsi,%rax),%ymm8
        vpaddw    (%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,(%rsi,%rdx)
/* and seven more like that */
        vmovdqa   32(%rsi,%rax),%ymm8
        vpaddw    32(%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,32(%rsi,%rdx)

        vmovdqa   64(%rsi,%rax),%ymm8
        vpaddw    64(%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,64(%rsi,%rdx)

        vmovdqa   96(%rsi,%rax),%ymm8
        vpaddw    96(%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,96(%rsi,%rdx)

        vmovdqa   128(%rsi,%rax),%ymm8
        vpaddw    128(%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,128(%rsi,%rdx)

        vmovdqa   160(%rsi,%rax),%ymm8
        vpaddw    160(%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,160(%rsi,%rdx)

        vmovdqa   192(%rsi,%rax),%ymm8
        vpaddw    192(%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,192(%rsi,%rdx)

        vmovdqa   224(%rsi,%rax),%ymm8
        vpaddw    224(%rsi,%rcx),%ymm8,%ymm8
        vpcmpgtw  %ymm1,%ymm8,%ymm2
        vpand     %ymm0,%ymm2,%ymm2
        vpsubw    %ymm2,%ymm8,%ymm8
        vmovdqa   %ymm8,224(%rsi,%rdx)

        addq      $3,%rdi   /* saves a mu-op to do it here */
        jmp       pc4asp1
pc4asp9:
        ret      
	.size	pc4asp, .-pc4asp

