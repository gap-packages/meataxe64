/* pcrits.s x86 assembler before AVX-512 changes*/
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15


// pc5aca rdi=prog rsi=bwa rdx=parms
// rax=program byte, r8=prog ptr  r9=place to store
// r10 slices left rcx slice stride

// unused r11
// untouchted rbp r12 r13 r14 r15

	.text
	.globl	pc5aca
pc5aca:                   /* initialization */
        movq        16(%rdx),%xmm9
        pshufd      $0x44,%xmm9,%xmm9
        movq         8(%rdx),%xmm10   /* shift S  */
        movq        24(%rdx),%xmm8
        pshufd      $0x44,%xmm8,%xmm8
        movq        0(%rdx),%xmm11
        pshufd      $0x44,%xmm11,%xmm11
        movq        48(%rdx),%r10     /* number of slices */
        movq        32(%rdx),%rcx     /* size of one slot */
        imul        40(%rdx),%rcx     /* times slots = slice stride */
// %rdx is actually spare from here on
pc5aca1:                   /* next slice */
        movq    %rdi,%r8   /* start at the program beginning */             
        movq    %rsi,%r9   /* set up place to store */
        addq    $256,%r9      /* destination starts at slot 2 */
        movdqa -128(%r9),%xmm0  /* load accumulator from slot 1 */
        movdqa -112(%r9),%xmm1
        movdqa -96(%r9),%xmm2
        movdqa -80(%r9),%xmm3
        movdqa -64(%r9),%xmm4
        movdqa -48(%r9),%xmm5
        movdqa -32(%r9),%xmm6
        movdqa -16(%r9),%xmm7
pc5aca2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pc5aca4       /* no - go see what it is */
pc5aca3:
        shlq    $7,%rax       /* convert to displacement */
        paddq    0(%rax,%rsi),%xmm0    /* add in cauldron */
        paddq   16(%rax,%rsi),%xmm1
        paddq   32(%rax,%rsi),%xmm2
        paddq   48(%rax,%rsi),%xmm3
        paddq   64(%rax,%rsi),%xmm4
        paddq   80(%rax,%rsi),%xmm5
        paddq   96(%rax,%rsi),%xmm6
        paddq  112(%rax,%rsi),%xmm7

        movdqa    %xmm0,%xmm12  
        movdqa    %xmm1,%xmm14 
        paddq   %xmm8,%xmm12  /* add 2^N - p */
        paddq   %xmm8,%xmm14
        pand    %xmm9,%xmm12  /* and with mask */
        pand    %xmm9,%xmm14 
        movdqa    %xmm12,%xmm13
        movdqa    %xmm14,%xmm15
        psrlq   %xmm10,%xmm13  /* subtract 1 if set */
        psrlq   %xmm10,%xmm15
        psubq   %xmm13,%xmm12
        psubq   %xmm15,%xmm14
        pand    %xmm11,%xmm12   /* and with p */
        pand    %xmm11,%xmm14
        psubq   %xmm12,%xmm0    /* subtract p if need be */
        psubq   %xmm14,%xmm1   
        movdqa  %xmm0,(%r9)
        movdqa  %xmm1,16(%r9)

        movdqa    %xmm2,%xmm12  
        movdqa    %xmm3,%xmm14 
        paddq   %xmm8,%xmm12
        paddq   %xmm8,%xmm14
        pand    %xmm9,%xmm12
        pand    %xmm9,%xmm14 
        movdqa    %xmm12,%xmm13
        movdqa    %xmm14,%xmm15
        psrlq   %xmm10,%xmm13 
        psrlq   %xmm10,%xmm15
        psubq   %xmm13,%xmm12
        psubq   %xmm15,%xmm14
        pand    %xmm11,%xmm12 
        pand    %xmm11,%xmm14
        psubq   %xmm12,%xmm2  
        psubq   %xmm14,%xmm3  
        movdqa  %xmm2,32(%r9)
        movdqa  %xmm3,48(%r9)

        movdqa    %xmm4,%xmm12  
        movdqa    %xmm5,%xmm14 
        paddq   %xmm8,%xmm12
        paddq   %xmm8,%xmm14
        pand    %xmm9,%xmm12
        pand    %xmm9,%xmm14 
        movdqa    %xmm12,%xmm13
        movdqa    %xmm14,%xmm15
        psrlq   %xmm10,%xmm13 
        psrlq   %xmm10,%xmm15
        psubq   %xmm13,%xmm12
        psubq   %xmm15,%xmm14
        pand    %xmm11,%xmm12 
        pand    %xmm11,%xmm14
        psubq   %xmm12,%xmm4  
        psubq   %xmm14,%xmm5  
        movdqa  %xmm4,64(%r9)
        movdqa  %xmm5,80(%r9)

        movdqa    %xmm6,%xmm12  
        movdqa    %xmm7,%xmm14 
        paddq   %xmm8,%xmm12
        paddq   %xmm8,%xmm14
        pand    %xmm9,%xmm12
        pand    %xmm9,%xmm14 
        movdqa    %xmm12,%xmm13
        movdqa    %xmm14,%xmm15
        psrlq   %xmm10,%xmm13 
        psrlq   %xmm10,%xmm15
        psubq   %xmm13,%xmm12
        psubq   %xmm15,%xmm14
        pand    %xmm11,%xmm12 
        pand    %xmm11,%xmm14
        psubq   %xmm12,%xmm6  
        psubq   %xmm14,%xmm7  
        movdqa  %xmm6,96(%r9)
        movdqa  %xmm7,112(%r9)

        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pc5aca3      /* yes - go straight round again */
pc5aca4:
        cmpq    $159,%rax
        ja      pc5aca5      /* not a load of accumulator either */
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
        jmp     pc5aca2
pc5aca5:
        cmpq    $239,%rax
        ja      pc5aca6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pc5aca2
pc5aca6:                      /* anything 240+ is stop at the moment */
        addq    %rcx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pc5aca1
        ret


// end of pc5aca

// pc5bmwa Afmt bwa Cfmt parms
// SSE 16-bit 

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pc5bmwa
pc5bmwa:
        pushq         %rbx
        movq        16(%rcx),%xmm8    /* mask     */
        pshufd      $0x44,%xmm8,%xmm8
        movq       8(%rcx),%xmm9      /* shift S  */
        movq        56(%rcx),%xmm10     /* 2^S % p  */
        pshufd      $0x44,%xmm10,%xmm10
        movq        64(%rcx),%xmm11     /* bias     */
        pshufd      $0x44,%xmm11,%xmm11 
        movq         32(%rcx),%r11     /* size of one slot */
        imul         40(%rcx),%r11     /* times slots = slice stride */

        movq    0(%rdi),%rax  /* first word of Afmt   */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        je      pc5bmwa8      /* yes get straight out */
//  Start of secondary loop
pc5bmwa1:
        shlq    $7,%r9        /* multiply by 128      */
        addq    %r9,%rdx      /* add into Cfmt addr   */
        movq    %rsi,%r8      /* copy BWA addr to %r8 */
        addq    $8,%rdi       /* point to next alcove */
        movq    $7,%rbx        /* number of slices  */

        movdqa   0(%rdx),%xmm0   /* get cauldron of Cfmt */
        movdqa  16(%rdx),%xmm1
        movdqa  32(%rdx),%xmm2
        movdqa  48(%rdx),%xmm3
        movdqa  64(%rdx),%xmm4
        movdqa  80(%rdx),%xmm5
        movdqa  96(%rdx),%xmm6
        movdqa 112(%rdx),%xmm7

//  Start of primary loop
pc5bmwa2:
        movq    %rax,%r9      /* copy Afmt to add index */
        shrq    $4,%r9        /* top nybble to bottom   */
        andq    $0x780,%r9     /* the nybble            */
        movq    %rax,%r10     /* copy Afmt to sub index */
        andq    $0x780,%r10     /* get bottom nybble      */
        shrq    $8,%rax       /* next byte of Afmt      */ 
        paddq    0(%r8,%r9),%xmm0    /* add in cauldron */
        paddq   16(%r8,%r9),%xmm1
        paddq   32(%r8,%r9),%xmm2
        paddq   48(%r8,%r9),%xmm3
        paddq   64(%r8,%r9),%xmm4
        paddq   80(%r8,%r9),%xmm5
        paddq   96(%r8,%r9),%xmm6
        paddq  112(%r8,%r9),%xmm7
        psubq    0(%r8,%r10),%xmm0
        psubq   16(%r8,%r10),%xmm1
        psubq   32(%r8,%r10),%xmm2
        psubq   48(%r8,%r10),%xmm3
        psubq   64(%r8,%r10),%xmm4
        psubq   80(%r8,%r10),%xmm5
        psubq   96(%r8,%r10),%xmm6
        psubq  112(%r8,%r10),%xmm7
        addq    %r11,%r8                 /* move on to next slice */
        subq    $1,%rbx
        jne     pc5bmwa2
//  End of primary loop

        movdqa  %xmm0,%xmm12
        movdqa  %xmm1,%xmm13
        movdqa  %xmm2,%xmm14
        movdqa  %xmm3,%xmm15
        pand    %xmm8,%xmm12
        pand    %xmm8,%xmm13
        pand    %xmm8,%xmm14
        pand    %xmm8,%xmm15
        pxor    %xmm12,%xmm0
        pxor    %xmm13,%xmm1
        pxor    %xmm14,%xmm2
        pxor    %xmm15,%xmm3
        psrld   %xmm9,%xmm12
        psrld   %xmm9,%xmm13
        psrld   %xmm9,%xmm14
        psrld   %xmm9,%xmm15
        pmullw  %xmm10,%xmm12
        pmullw  %xmm10,%xmm13
        pmullw  %xmm10,%xmm14
        pmullw  %xmm10,%xmm15
        paddq   %xmm12,%xmm0
        paddq   %xmm13,%xmm1
        paddq   %xmm14,%xmm2
        paddq   %xmm15,%xmm3
        paddq   %xmm11,%xmm0
        paddq   %xmm11,%xmm1
        paddq   %xmm11,%xmm2
        paddq   %xmm11,%xmm3
        movdqa  %xmm0,0(%rdx)
        movdqa  %xmm1,16(%rdx)
        movdqa  %xmm2,32(%rdx)
        movdqa  %xmm3,48(%rdx)

        movdqa  %xmm4,%xmm12
        movdqa  %xmm5,%xmm13
        movdqa  %xmm6,%xmm14
        movdqa  %xmm7,%xmm15
        pand    %xmm8,%xmm12
        pand    %xmm8,%xmm13
        pand    %xmm8,%xmm14
        pand    %xmm8,%xmm15
        pxor    %xmm12,%xmm4
        pxor    %xmm13,%xmm5
        pxor    %xmm14,%xmm6
        pxor    %xmm15,%xmm7
        psrld   %xmm9,%xmm12
        psrld   %xmm9,%xmm13
        psrld   %xmm9,%xmm14
        psrld   %xmm9,%xmm15
        pmullw  %xmm10,%xmm12
        pmullw  %xmm10,%xmm13
        pmullw  %xmm10,%xmm14
        pmullw  %xmm10,%xmm15
        paddq   %xmm12,%xmm4
        paddq   %xmm13,%xmm5
        paddq   %xmm14,%xmm6
        paddq   %xmm15,%xmm7
        paddq   %xmm11,%xmm4
        paddq   %xmm11,%xmm5
        paddq   %xmm11,%xmm6
        paddq   %xmm11,%xmm7
        movdqa  %xmm4,64(%rdx)
        movdqa  %xmm5,80(%rdx)
        movdqa  %xmm6,96(%rdx)
        movdqa  %xmm7,112(%rdx)

        movq    0(%rdi),%rax  /* next word of Afmt    */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        jne     pc5bmwa1         /* no - round again     */
//  End of secondary loop
pc5bmwa8:
        popq    %rbx
        ret      

// end of pc5bmwa

// pc5bmdd Afmt bwa Cfmt parms
// SSE pmulld slower but can do 10-bit

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pc5bmdd
pc5bmdd:
        pushq         %rbx
        movq        16(%rcx),%xmm8    /* mask     */
        pshufd      $0x44,%xmm8,%xmm8
        movq       8(%rcx),%xmm9      /* shift S  */
        movq        56(%rcx),%xmm10     /* 2^S % p  */
        pshufd      $0x44,%xmm10,%xmm10
        movq        64(%rcx),%xmm11     /* bias     */
        pshufd      $0x44,%xmm11,%xmm11 
        movq         32(%rcx),%r11     /* size of one slot */
        imul         40(%rcx),%r11     /* times slots = slice stride */

        movq    0(%rdi),%rax  /* first word of Afmt   */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        je      pc5bmdd8      /* yes get straight out */
//  Start of secondary loop
pc5bmdd1:
        shlq    $7,%r9        /* multiply by 128      */
        addq    %r9,%rdx      /* add into Cfmt addr   */
        movq    %rsi,%r8      /* copy BWA addr to %r8 */
        addq    $8,%rdi       /* point to next alcove */
        movq    $7,%rbx        /* number of slices  */

        movdqa   0(%rdx),%xmm0   /* get cauldron of Cfmt */
        movdqa  16(%rdx),%xmm1
        movdqa  32(%rdx),%xmm2
        movdqa  48(%rdx),%xmm3
        movdqa  64(%rdx),%xmm4
        movdqa  80(%rdx),%xmm5
        movdqa  96(%rdx),%xmm6
        movdqa 112(%rdx),%xmm7

//  Start of primary loop
pc5bmdd2:
        movq    %rax,%r9      /* copy Afmt to add index */
        shrq    $4,%r9        /* top nybble to bottom   */
        andq    $0x780,%r9     /* the nybble            */
        movq    %rax,%r10     /* copy Afmt to sub index */
        andq    $0x780,%r10     /* get bottom nybble      */
        shrq    $8,%rax       /* next byte of Afmt      */ 
        paddq    0(%r8,%r9),%xmm0    /* add in cauldron */
        paddq   16(%r8,%r9),%xmm1
        paddq   32(%r8,%r9),%xmm2
        paddq   48(%r8,%r9),%xmm3
        paddq   64(%r8,%r9),%xmm4
        paddq   80(%r8,%r9),%xmm5
        paddq   96(%r8,%r9),%xmm6
        paddq  112(%r8,%r9),%xmm7
        psubq    0(%r8,%r10),%xmm0
        psubq   16(%r8,%r10),%xmm1
        psubq   32(%r8,%r10),%xmm2
        psubq   48(%r8,%r10),%xmm3
        psubq   64(%r8,%r10),%xmm4
        psubq   80(%r8,%r10),%xmm5
        psubq   96(%r8,%r10),%xmm6
        psubq  112(%r8,%r10),%xmm7
        addq    %r11,%r8                 /* move on to next slice */
        subq    $1,%rbx
        jne     pc5bmdd2
//  End of primary loop

        movdqa  %xmm0,%xmm12
        movdqa  %xmm1,%xmm13
        movdqa  %xmm2,%xmm14
        movdqa  %xmm3,%xmm15
        pand    %xmm8,%xmm12
        pand    %xmm8,%xmm13
        pand    %xmm8,%xmm14
        pand    %xmm8,%xmm15
        pxor    %xmm12,%xmm0
        pxor    %xmm13,%xmm1
        pxor    %xmm14,%xmm2
        pxor    %xmm15,%xmm3
        psrld   %xmm9,%xmm12
        psrld   %xmm9,%xmm13
        psrld   %xmm9,%xmm14
        psrld   %xmm9,%xmm15
        pmulld  %xmm10,%xmm12
        pmulld  %xmm10,%xmm13
        pmulld  %xmm10,%xmm14
        pmulld  %xmm10,%xmm15
        paddq   %xmm12,%xmm0
        paddq   %xmm13,%xmm1
        paddq   %xmm14,%xmm2
        paddq   %xmm15,%xmm3
        paddq   %xmm11,%xmm0
        paddq   %xmm11,%xmm1
        paddq   %xmm11,%xmm2
        paddq   %xmm11,%xmm3
        movdqa  %xmm0,0(%rdx)
        movdqa  %xmm1,16(%rdx)
        movdqa  %xmm2,32(%rdx)
        movdqa  %xmm3,48(%rdx)

        movdqa  %xmm4,%xmm12
        movdqa  %xmm5,%xmm13
        movdqa  %xmm6,%xmm14
        movdqa  %xmm7,%xmm15
        pand    %xmm8,%xmm12
        pand    %xmm8,%xmm13
        pand    %xmm8,%xmm14
        pand    %xmm8,%xmm15
        pxor    %xmm12,%xmm4
        pxor    %xmm13,%xmm5
        pxor    %xmm14,%xmm6
        pxor    %xmm15,%xmm7
        psrld   %xmm9,%xmm12
        psrld   %xmm9,%xmm13
        psrld   %xmm9,%xmm14
        psrld   %xmm9,%xmm15
        pmulld  %xmm10,%xmm12
        pmulld  %xmm10,%xmm13
        pmulld  %xmm10,%xmm14
        pmulld  %xmm10,%xmm15
        paddq   %xmm12,%xmm4
        paddq   %xmm13,%xmm5
        paddq   %xmm14,%xmm6
        paddq   %xmm15,%xmm7
        paddq   %xmm11,%xmm4
        paddq   %xmm11,%xmm5
        paddq   %xmm11,%xmm6
        paddq   %xmm11,%xmm7
        movdqa  %xmm4,64(%rdx)
        movdqa  %xmm5,80(%rdx)
        movdqa  %xmm6,96(%rdx)
        movdqa  %xmm7,112(%rdx)

        movq    0(%rdi),%rax  /* next word of Afmt    */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        jne     pc5bmdd1         /* no - round again     */
//  End of secondary loop
pc5bmdd8:
        popq    %rbx
        ret      

// end of pc5bmdd


// pc5bmwj Afmt bwa Cfmt parms
// AVX2 vpmullw 16-bit faster but no 10-bit primes

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pc5bmwj
pc5bmwj:
        pushq         %rbx
        vpbroadcastq 16(%rcx),%ymm8    /* mask     */
        vmovq         8(%rcx),%xmm9    /* shift S  */
        vpbroadcastq 56(%rcx),%ymm10   /* 2^S % p  */
        vpbroadcastq 64(%rcx),%ymm11   /* bias     */
        movq         32(%rcx),%r11     /* size of one slot */
        imul         40(%rcx),%r11     /* times slots = slice stride */

        movq    0(%rdi),%rax  /* first word of Afmt   */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        je      pc5bmwj5        /* yes get straight out */
//  Start of secondary loop
pc5bmwj1:
        shlq    $7,%r9        /* multiply by 128      */
        addq    %r9,%rdx      /* add into Cfmt addr   */
        movq    %rsi,%r8      /* copy BWA addr to %r8 */
        addq    $8,%rdi       /* point to next alcove */
        movq    $7,%rbx        /* number of slices  */
        vmovdqa  0(%rdx),%ymm0   /* get cauldron of Cfmt */
        vmovdqa 32(%rdx),%ymm1
        vmovdqa 64(%rdx),%ymm2
        vmovdqa 96(%rdx),%ymm3

//  Start of primary loop
pc5bmwj2:
   
        movq    %rax,%r9      /* copy Afmt to add index */
        shrq    $4,%r9        /* top nybble to bottom   */
        andq    $0x780,%r9     /* the nybble            */
        movq    %rax,%r10     /* copy Afmt to sub index */
        andq    $0x780,%r10     /* get bottom nybble      */
        shrq    $8,%rax       /* next byte of Afmt      */ 
        vpaddq   0(%r8,%r9),%ymm0,%ymm0  /* add in cauldron */
        vpaddq  32(%r8,%r9),%ymm1,%ymm1
        vpaddq  64(%r8,%r9),%ymm2,%ymm2
        vpaddq  96(%r8,%r9),%ymm3,%ymm3
        vpsubq   0(%r8,%r10),%ymm0,%ymm0  /* subtract cauldron */
        vpsubq  32(%r8,%r10),%ymm1,%ymm1
        vpsubq  64(%r8,%r10),%ymm2,%ymm2
        vpsubq  96(%r8,%r10),%ymm3,%ymm3
        addq    %r11,%r8                 /* move on to next slice */
        subq    $1,%rbx
        jne     pc5bmwj2
//  End of primary loop
        vpand   %ymm8,%ymm0,%ymm4   /* get top bits of each */
        vpand   %ymm8,%ymm1,%ymm5
        vpand   %ymm8,%ymm2,%ymm6
        vpand   %ymm8,%ymm3,%ymm7
        vpxor   %ymm4,%ymm0,%ymm0   /* xor them back in     */
        vpxor   %ymm5,%ymm1,%ymm1
        vpxor   %ymm6,%ymm2,%ymm2
        vpxor   %ymm7,%ymm3,%ymm3
        vpsrld  %xmm9,%ymm4,%ymm4     /* divide by 2^S     */
        vpsrld  %xmm9,%ymm5,%ymm5
        vpsrld  %xmm9,%ymm6,%ymm6
        vpsrld  %xmm9,%ymm7,%ymm7
        vpmullw %ymm10,%ymm4,%ymm4   /* multiply by (2^S)%p   */
        vpmullw %ymm10,%ymm5,%ymm5
        vpmullw %ymm10,%ymm6,%ymm6
        vpmullw %ymm10,%ymm7,%ymm7
        vpaddq  %ymm4,%ymm0,%ymm0         /* and add that in      */
        vpaddq  %ymm5,%ymm1,%ymm1
        vpaddq  %ymm6,%ymm2,%ymm2
        vpaddq  %ymm7,%ymm3,%ymm3
        vpaddq  %ymm11,%ymm0,%ymm0        /* add in bias          */
        vpaddq  %ymm11,%ymm1,%ymm1
        vpaddq  %ymm11,%ymm2,%ymm2
        vpaddq  %ymm11,%ymm3,%ymm3
        vmovdqa %ymm0,0(%rdx)   /* Put Cfmt cauldron back */
        vmovdqa %ymm1,32(%rdx)
        vmovdqa %ymm2,64(%rdx)
        vmovdqa %ymm3,96(%rdx)
        movq    0(%rdi),%rax  /* next word of Afmt    */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        jne     pc5bmwj1         /* no - round again     */
//  End of secondary loop
pc5bmwj5:
        popq    %rbx
        ret      

// end of pc5bmwj

// pc5bmdj Afmt bwa Cfmt parms
// AVX2 vpmulld (slower but can do 10-bit primes)

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pc5bmdj
pc5bmdj:
        pushq         %rbx
        vpbroadcastq 16(%rcx),%ymm8    /* mask     */
        vmovq         8(%rcx),%xmm9    /* shift S  */
        vpbroadcastq 56(%rcx),%ymm10   /* 2^S % p  */
        vpbroadcastq 64(%rcx),%ymm11   /* bias     */
        movq         32(%rcx),%r11     /* size of one slot */
        imul         40(%rcx),%r11     /* times slots = slice stride */

        movq    0(%rdi),%rax  /* first word of Afmt   */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        je      pc5bmdj5        /* yes get straight out */
//  Start of secondary loop
pc5bmdj1:
        shlq    $7,%r9        /* multiply by 128      */
        addq    %r9,%rdx      /* add into Cfmt addr   */
        movq    %rsi,%r8      /* copy BWA addr to %r8 */
        addq    $8,%rdi       /* point to next alcove */
        movq    $7,%rbx        /* number of slices  */
        vmovdqa  0(%rdx),%ymm0   /* get cauldron of Cfmt */
        vmovdqa 32(%rdx),%ymm1
        vmovdqa 64(%rdx),%ymm2
        vmovdqa 96(%rdx),%ymm3

//  Start of primary loop
pc5bmdj2:
   
        movq    %rax,%r9      /* copy Afmt to add index */
        shrq    $4,%r9        /* top nybble to bottom   */
        andq    $0x780,%r9     /* the nybble            */
        movq    %rax,%r10     /* copy Afmt to sub index */
        andq    $0x780,%r10     /* get bottom nybble      */
        shrq    $8,%rax       /* next byte of Afmt      */ 
        vpaddq   0(%r8,%r9),%ymm0,%ymm0  /* add in cauldron */
        vpaddq  32(%r8,%r9),%ymm1,%ymm1
        vpaddq  64(%r8,%r9),%ymm2,%ymm2
        vpaddq  96(%r8,%r9),%ymm3,%ymm3
        vpsubq   0(%r8,%r10),%ymm0,%ymm0  /* subtract cauldron */
        vpsubq  32(%r8,%r10),%ymm1,%ymm1
        vpsubq  64(%r8,%r10),%ymm2,%ymm2
        vpsubq  96(%r8,%r10),%ymm3,%ymm3
        addq    %r11,%r8                 /* move on to next slice */
        subq    $1,%rbx
        jne     pc5bmdj2
//  End of primary loop
        vpand   %ymm8,%ymm0,%ymm4   /* get top bits of each */
        vpand   %ymm8,%ymm1,%ymm5
        vpand   %ymm8,%ymm2,%ymm6
        vpand   %ymm8,%ymm3,%ymm7
        vpxor   %ymm4,%ymm0,%ymm0   /* xor them back in     */
        vpxor   %ymm5,%ymm1,%ymm1
        vpxor   %ymm6,%ymm2,%ymm2
        vpxor   %ymm7,%ymm3,%ymm3
        vpsrld  %xmm9,%ymm4,%ymm4     /* divide by 2^S     */
        vpsrld  %xmm9,%ymm5,%ymm5
        vpsrld  %xmm9,%ymm6,%ymm6
        vpsrld  %xmm9,%ymm7,%ymm7
        vpmulld %ymm10,%ymm4,%ymm4   /* multiply by (2^S)%p   */
        vpmulld %ymm10,%ymm5,%ymm5
        vpmulld %ymm10,%ymm6,%ymm6
        vpmulld %ymm10,%ymm7,%ymm7
        vpaddq  %ymm4,%ymm0,%ymm0         /* and add that in      */
        vpaddq  %ymm5,%ymm1,%ymm1
        vpaddq  %ymm6,%ymm2,%ymm2
        vpaddq  %ymm7,%ymm3,%ymm3
        vpaddq  %ymm11,%ymm0,%ymm0        /* add in bias          */
        vpaddq  %ymm11,%ymm1,%ymm1
        vpaddq  %ymm11,%ymm2,%ymm2
        vpaddq  %ymm11,%ymm3,%ymm3
        vmovdqa %ymm0,0(%rdx)   /* Put Cfmt cauldron back */
        vmovdqa %ymm1,32(%rdx)
        vmovdqa %ymm2,64(%rdx)
        vmovdqa %ymm3,96(%rdx)
        movq    0(%rdi),%rax  /* next word of Afmt    */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        jne     pc5bmdj1         /* no - round again     */
//  End of secondary loop
pc5bmdj5:
        popq    %rbx
        ret      

// end of pc5bmdj




// pc5bmdm Afmt bwa Cfmt parms
// AVX2 vpmulld (slower but can do 10-bit primes)

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pc5bmdm
pc5bmdm:
        pushq         %rbx
        vpbroadcastq 16(%rcx),%zmm8    /* mask     */
        vmovq         8(%rcx),%xmm9    /* shift S  */
        vpbroadcastq 56(%rcx),%zmm10   /* 2^S % p  */
        vpbroadcastq 64(%rcx),%zmm11   /* bias     */
        movq         32(%rcx),%r11     /* size of one slot */
        imul         40(%rcx),%r11     /* times slots = slice stride */

        movq    0(%rdi),%rax  /* first word of Afmt   */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        je      pc5bmdm5        /* yes get straight out */
//  Start of secondary loop
pc5bmdm1:
        shlq    $7,%r9        /* multiply by 128      */
        addq    %r9,%rdx      /* add into Cfmt addr   */
        movq    %rsi,%r8      /* copy BWA addr to %r8 */
        addq    $8,%rdi       /* point to next alcove */
        movq    $7,%rbx        /* number of slices  */
        vmovdqa64  0(%rdx),%zmm0   /* get cauldron of Cfmt */
        vmovdqa64 64(%rdx),%zmm2

//  Start of primary loop
pc5bmdm2:
   
        movq    %rax,%r9      /* copy Afmt to add index */
        shrq    $4,%r9        /* top nybble to bottom   */
        andq    $0x780,%r9     /* the nybble            */
        movq    %rax,%r10     /* copy Afmt to sub index */
        andq    $0x780,%r10     /* get bottom nybble      */
        shrq    $8,%rax       /* next byte of Afmt      */ 
        vpaddq   0(%r8,%r9),%zmm0,%zmm0  /* add in cauldron */
        vpaddq  64(%r8,%r9),%zmm2,%zmm2
        vpsubq   0(%r8,%r10),%zmm0,%zmm0  /* subtract cauldron */
        vpsubq  64(%r8,%r10),%zmm2,%zmm2
        addq    %r11,%r8                 /* move on to next slice */
        subq    $1,%rbx
        jne     pc5bmdm2
//  End of primary loop
        vpandd   %zmm8,%zmm0,%zmm4   /* get top bits of each */
        vpandd   %zmm8,%zmm2,%zmm6
        vpxord   %zmm4,%zmm0,%zmm0   /* xor them back in     */
        vpxord   %zmm6,%zmm2,%zmm2
        vpsrld  %xmm9,%zmm4,%zmm4     /* divide by 2^S     */
        vpsrld  %xmm9,%zmm6,%zmm6
        vpmulld %zmm10,%zmm4,%zmm4   /* multiply by (2^S)%p   */
        vpmulld %zmm10,%zmm6,%zmm6
        vpaddq  %zmm4,%zmm0,%zmm0         /* and add that in      */
        vpaddq  %zmm6,%zmm2,%zmm2
        vpaddq  %zmm11,%zmm0,%zmm0        /* add in bias          */
        vpaddq  %zmm11,%zmm2,%zmm2
        vmovdqa64 %zmm0,0(%rdx)   /* Put Cfmt cauldron back */
        vmovdqa64 %zmm2,64(%rdx)
        movq    0(%rdi),%rax  /* next word of Afmt    */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        jne     pc5bmdm1         /* no - round again     */
//  End of secondary loop
pc5bmdm5:
        popq    %rbx
        ret      

// end of pc5bmdm


