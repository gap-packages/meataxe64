/* pcrit2s.s assembler for AVX-2 version */
/* R. A. Parker 10.10.2017  */
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15


// pcbmdq Afmt bwa Cfmt parms

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */

	.text
	.globl	pcbmdq
	.type	pcbmdq, @function
pcbmdq:
//  get eight C-values into %ymm1-8
        movq     $64,%rcx
pcbmdq5:
//  get all four Aij values into %ymm9
//  get 32-bit Bj1,Bj2 values into %ymm11,12
        vpermq   $0,%ymm9,%ymm10       /*  get a 32-bit A1j into %ymm10  */
        vpmuludq %ymm10,%ymm11,%ymm15  /*  multiply by Bj1  */
        vpaddq  %ymm15,%ymm1,%ymm1     /*  add into C11     */
        vpmuludq %ymm10,%ymm12,%ymm15  /*  multiply by Bj2  */
        vpaddq  %ymm15,%ymm2,%ymm2     /*  add into C12     */
        vpermq   $0x55,%ymm9,%ymm10    /*  get a 32-bit A2j into %ymm10  */
        vpmuludq %ymm10,%ymm11,%ymm15
        vpaddq  %ymm15,%ymm3,%ymm3
        vpmuludq %ymm10,%ymm12,%ymm15
        vpaddq  %ymm15,%ymm4,%ymm4
        vpermq   $0xAA,%ymm9,%ymm10    /*  get a 32-bit A3j into %ymm10  */
        vpmuludq %ymm10,%ymm11,%ymm15
        vpaddq  %ymm15,%ymm5,%ymm5
        vpmuludq %ymm10,%ymm12,%ymm15
        vpaddq  %ymm15,%ymm6,%ymm6
        vpermq   $0xff,%ymm9,%ymm10    /*  get a 32-bit A4j into %ymm10  */
        vpmuludq %ymm10,%ymm11,%ymm15
        vpaddq  %ymm15,%ymm7,%ymm7
        vpmuludq %ymm10,%ymm12,%ymm15
        vpaddq  %ymm15,%ymm8,%ymm8
        subq     $1,%rcx
        jne      pcbmdq5
        ret
	.size	pcbmdq, .-pcbmdq

// end of pcbmdq

// pcchain rdi=prog rsi=bwa rdx=parms
// rax=program byte, r8=prog ptr  r9=place to store
// r10 slices left rcx slice stride

// unused r11
// untouchted rbp r12 r13 r14 r15

	.text
	.globl	pcchain
	.type	pcchain, @function
pcchain:                   /* initialization */
        vpbroadcastq 16(%rdx),%ymm9    /* mask     */
        vmovq         8(%rdx),%xmm10   /* shift S  */
        vpbroadcastq 24(%rdx),%ymm8    /* 2^S - p  */
        vpbroadcastq  0(%rdx),%ymm11   /* p  */
        movq         48(%rdx),%r10     /* number of slices */
        movq         32(%rdx),%rcx     /* size of one slot */
        imul         40(%rdx),%rcx     /* times slots = slice stride */
// %rdx is actually spare from here on
pcchain1:                  /* next slice */
        movq    %rdi,%r8   /* start at the program beginning */             
        movq    %rsi,%r9   /* set up place to store */
        addq    $256,%r9      /* destination starts at slot 2 */
        vmovdqa -128(%r9),%ymm0  /* load accumulator from slot 1 */
        vmovdqa -96(%r9),%ymm1
        vmovdqa -64(%r9),%ymm2
        vmovdqa -32(%r9),%ymm3
pcchain2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pcchain4      /* no - go see what it is */
pcchain3:
        shlq    $7,%rax       /* convert to displacement */
        vpaddq   0(%rax,%rsi),%ymm0,%ymm0  /* add in cauldron */
        vpaddq  32(%rax,%rsi),%ymm1,%ymm1
        vpaddq  64(%rax,%rsi),%ymm2,%ymm2
        vpaddq  96(%rax,%rsi),%ymm3,%ymm3
        vpaddq  %ymm8,%ymm0,%ymm4          /* add 2^N - p */
        vpaddq  %ymm8,%ymm1,%ymm5
        vpaddq  %ymm8,%ymm2,%ymm6
        vpaddq  %ymm8,%ymm3,%ymm7
        vpand   %ymm9,%ymm4,%ymm4          /* and with mask */
        vpand   %ymm9,%ymm5,%ymm5
        vpand   %ymm9,%ymm6,%ymm6
        vpand   %ymm9,%ymm7,%ymm7
        vpsrld  %xmm10,%ymm4,%ymm12        /* subtract 1 if set */
        vpsrld  %xmm10,%ymm5,%ymm13 
        vpsrld  %xmm10,%ymm6,%ymm14     /* think this should be quadword */
        vpsrld  %xmm10,%ymm7,%ymm15 
        vpsubq  %ymm12,%ymm4,%ymm4
        vpsubq  %ymm13,%ymm5,%ymm5
        vpsubq  %ymm14,%ymm6,%ymm6
        vpsubq  %ymm15,%ymm7,%ymm7
        vpand   %ymm11,%ymm4,%ymm4         /* and with p */
        vpand   %ymm11,%ymm5,%ymm5
        vpand   %ymm11,%ymm6,%ymm6
        vpand   %ymm11,%ymm7,%ymm7
        vpsubq  %ymm4,%ymm0,%ymm0          /* subtract p if need be */
        vpsubq  %ymm5,%ymm1,%ymm1
        vpsubq  %ymm6,%ymm2,%ymm2
        vpsubq  %ymm7,%ymm3,%ymm3
        vmovdqa %ymm0,0(%r9)   /* Store result in BWA */
        vmovdqa %ymm1,32(%r9)
        vmovdqa %ymm2,64(%r9)
        vmovdqa %ymm3,96(%r9)
        addq    $128,%r9      /* increment destination slot */
        movzbq  0(%r8),%rax   /* get next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        jbe     pcchain3      /* yes - go straight round again */
pcchain4:
        cmpq    $159,%rax
        ja      pcchain5      /* not a load of accumulator either */
        subq    $80,%rax
        shlq    $7,%rax        /* multiply by slot size */
        vmovdqa 0(%rax,%rsi),%ymm0
        vmovdqa 32(%rax,%rsi),%ymm1
        vmovdqa 64(%rax,%rsi),%ymm2
        vmovdqa 96(%rax,%rsi),%ymm3
        jmp     pcchain2
pcchain5:
        cmpq    $239,%rax
        ja      pcchain6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pcchain2
pcchain6:                      /* anything 240+ is stop at the moment */
        addq    %rcx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pcchain1
        ret

	.size	pcchain, .-pcchain

// end of pcchain

// pcbmas Afmt bwa Cfmt parms

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pcbmas
	.type	pcbmas, @function
pcbmas:
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
        je      getoutas      /* yes get straight out */
//  Start of secondary loop
pcas1:
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
pcas2:
   
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
        jne     pcas2
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
        jne     pcas1         /* no - round again     */
//  End of secondary loop
getoutas:
        popq    %rbx
        ret      
	.size	pcbmas, .-pcbmas

// end of pcbmas

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
        movq    %rdx,%r9
        cmpq    $8,%r9
        jb      pcxunf6
pcxunf1:
        movq    (%rsi),%rdx    /* get unary input */
        movzbq  %dl,%rax
        movzbq  (%rcx,%rax),%r8
        movzbl  %dh,%eax
        shrq    $16,%rdx
        movzbq  (%rcx,%rax),%rax
        shlq    $8,%rax
        addq    %rax,%r8
        movzbq  %dl,%rax 
        movzbq  (%rcx,%rax),%rax
        shlq    $16,%rax
        addq    %rax,%r8
        movzbl  %dh,%eax 
        shrq    $16,%rdx
        movzbq  (%rcx,%rax),%rax
        shlq    $24,%rax
        addq    %rax,%r8
        movzbq  %dl,%rax 
        movzbq  (%rcx,%rax),%rax
        shlq    $32,%rax
        addq    %rax,%r8
        movzbl  %dh,%eax 
        shrq    $16,%rdx
        movzbq  (%rcx,%rax),%rax
        shlq    $40,%rax
        addq    %rax,%r8
        movzbq  %dl,%rax 
        movzbq  (%rcx,%rax),%rax
        shlq    $48,%rax
        addq    %rax,%r8
        movzbl  %dh,%eax 
        movzbq  (%rcx,%rax),%rax
        shlq    $56,%rax
        addq    %rax,%r8
        xorq    %r8,(%rdi)
        addq    $8,%rdi
        addq    $8,%rsi
        subq    $8,%r9
        cmpq    $8,%r9
        jae     pcxunf1

pcxunf6:
        cmpq    $0,%r9
        je      pcxunf8
pcxunf7:
        movzbq  (%rsi),%rax  
        movb    (%rcx,%rax),%ah
        xorb    %ah,(%rdi)
        addq    $1,%rdi
        addq    $1,%rsi
        subq    $1,%r9
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

/* end of pcritHASs.s  */
