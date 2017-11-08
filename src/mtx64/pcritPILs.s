/* pcrit1s.s assembler for SSE, Piledriver version */
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

// pccl32 parms scalar noc  d1  d2
//         rdi   rsi   rdx rcx  r8

	.text
	.globl	pccl32
	.type	pccl32, @function
// initialization
pccl32:
       vpxor    %xmm0,%xmm0,%xmm0     /* %xmm0 just contains zero */
       vpinsrq  $0,%rsi,%xmm0,%xmm2   /* scalar from %rsi */
       vpinsrq  $0,0(%rdi),%xmm0,%xmm4 /* e left justified */
       vpinsrq  $0,8(%rdi),%xmm0,%xmm6 /* p left justified*/
       vpinsrq  $0,16(%rdi),%xmm0,%xmm8 /* shift left - right justified */
pccl32p:
        vpinsrd     $0,(%rcx),%xmm0,%xmm1    /* get 32-bit quantity */
        vpclmulqdq  $0,%xmm1,%xmm2,%xmm3   /* multiply by scalar */
        vpsllq      $1,%xmm3,%xmm7
        vpclmulqdq  $16,%xmm7,%xmm4, %xmm5  /* multiply high part a by e */
        vpsllq      $1,%xmm5,%xmm5
        vpclmulqdq  $16,%xmm5,%xmm6,%xmm7   /* multiply high part s by p */
        vpxor       %xmm3,%xmm7,%xmm7       /* xor left justified parts */
        vpinsrd     $0,(%r8),%xmm0,%xmm1    /* get other 32-bit quantity */
        vpsrlq      %xmm8,%xmm7,%xmm7    /* shift into registration */
        vpxor       %xmm7,%xmm1,%xmm1    /* add that in too */
        vpextrd     $0,%xmm1,(%r8)       /* store the result back */
        addq        $4,%rcx
        addq        $4,%r8
        subq        $1,%rdx
        jne         pccl32p
        ret
	.size	pccl32, .-pccl32

// end of pccl32

// pccl64 parms scalar noc  d1  d2
//         rdi   rsi   rdx rcx  r8

	.text
	.globl	pccl64
	.type	pccl64, @function
// initialization
pccl64:
       vpxor    %xmm0,%xmm0,%xmm0     /* %xmm0 just contains zero */
       vpinsrq  $0,%rsi,%xmm0,%xmm2   /* scalar from %rsi */
       vpinsrq  $0,0(%rdi),%xmm0,%xmm4 /* e left justified */
       vpinsrq  $0,8(%rdi),%xmm0,%xmm6 /* p left justified*/
       vpinsrq  $0,16(%rdi),%xmm0,%xmm8 /* shift left - right justified */
pccl64p:
        vpinsrq     $0,(%rcx),%xmm0,%xmm1    /* get 64-bit quantity */
        vpclmulqdq  $0,%xmm1,%xmm2,%xmm3   /* multiply by scalar */
        vpsllq      $1,%xmm3,%xmm7
        vpclmulqdq  $16,%xmm7,%xmm4, %xmm5  /* multiply high part a by e */
        vpsllq      $1,%xmm5,%xmm5
        vpclmulqdq  $16,%xmm5,%xmm6,%xmm7   /* multiply high part s by p */
        vpxor       %xmm3,%xmm7,%xmm7       /* xor left justified parts */
        vpinsrq     $0,(%r8),%xmm0,%xmm1    /* get other 64-bit quantity */
        vpsrlq      %xmm8,%xmm7,%xmm7    /* shift into registration */
        vpxor       %xmm7,%xmm1,%xmm1    /* add that in too */
        vpextrq     $0,%xmm1,(%r8)       /* store the result back */
        addq        $8,%rcx
        addq        $8,%r8
        subq        $1,%rdx
        jne         pccl64p
        ret
	.size	pccl64, .-pccl64

// end of pccl64


// pcchain rdi=prog rsi=bwa rdx=parms
// rax=program byte, r8=prog ptr  r9=place to store
// r10 slices left rcx slice stride

// unused r11
// untouchted rbp r12 r13 r14 r15

	.text
	.globl	pcchain
	.type	pcchain, @function
pcchain:                   /* initialization */
        vmovq       16(%rdx),%xmm9
        vpinsrq     $1,16(%rdx),%xmm9,%xmm9
        vmovq       8(%rdx),%xmm10   /* shift S  */
        vmovq       24(%rdx),%xmm8
        vpinsrq     $1,24(%rdx),%xmm8,%xmm8
        vmovq       0(%rdx),%xmm11
        vpinsrq     $1,0(%rdx),%xmm11,%xmm11
        movq        48(%rdx),%r10     /* number of slices */
        movq        32(%rdx),%rcx     /* size of one slot */
        imul        40(%rdx),%rcx     /* times slots = slice stride */
// %rdx is actually spare from here on
pcchain1:                  /* next slice */
        movq    %rdi,%r8   /* start at the program beginning */             
        movq    %rsi,%r9   /* set up place to store */
        addq    $256,%r9      /* destination starts at slot 2 */
        vmovdqa -128(%r9),%xmm0  /* load accumulator from slot 1 */
        vmovdqa -112(%r9),%xmm1
        vmovdqa -96(%r9),%xmm2
        vmovdqa -80(%r9),%xmm3
        vmovdqa -64(%r9),%xmm4
        vmovdqa -48(%r9),%xmm5
        vmovdqa -32(%r9),%xmm6
        vmovdqa -16(%r9),%xmm7
pcchain2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pcchain4      /* no - go see what it is */
pcchain3:
        shlq    $7,%rax       /* convert to displacement */
        vpaddq    0(%rax,%rsi),%xmm0,%xmm0    /* add in cauldron */
        vpaddq   16(%rax,%rsi),%xmm1,%xmm1
        vpaddq   32(%rax,%rsi),%xmm2,%xmm2
        vpaddq   48(%rax,%rsi),%xmm3,%xmm3
        vpaddq   64(%rax,%rsi),%xmm4,%xmm4
        vpaddq   80(%rax,%rsi),%xmm5,%xmm5
        vpaddq   96(%rax,%rsi),%xmm6,%xmm6
        vpaddq  112(%rax,%rsi),%xmm7,%xmm7

        vpaddq   %xmm8,%xmm0,%xmm12  /* add 2^N - p */
        vpaddq   %xmm8,%xmm1,%xmm14
        vpand    %xmm9,%xmm12,%xmm12  /* and with mask */
        vpand    %xmm9,%xmm14,%xmm14 
        vpsrlq  %xmm10,%xmm12,%xmm13  /* subtract 1 if set */
        vpsrlq  %xmm10,%xmm14,%xmm15
        vpsubq   %xmm13,%xmm12,%xmm12
        vpsubq   %xmm15,%xmm14,%xmm14
        vpand    %xmm11,%xmm12,%xmm12   /* and with p */
        vpand    %xmm11,%xmm14,%xmm14
        vpsubq   %xmm12,%xmm0,%xmm0    /* subtract p if need be */
        vpsubq   %xmm14,%xmm1,%xmm1   
        vmovdqa  %xmm0,(%r9)
        vmovdqa  %xmm1,16(%r9)
 
        vpaddq   %xmm8,%xmm2,%xmm12
        vpaddq   %xmm8,%xmm3,%xmm14
        vpand    %xmm9,%xmm12,%xmm12
        vpand    %xmm9,%xmm14,%xmm14
        vpsrlq   %xmm10,%xmm12,%xmm13 
        vpsrlq   %xmm10,%xmm14,%xmm15
        vpsubq   %xmm13,%xmm12,%xmm12
        vpsubq   %xmm15,%xmm14,%xmm14
        vpand    %xmm11,%xmm12,%xmm12
        vpand    %xmm11,%xmm14,%xmm14
        vpsubq   %xmm12,%xmm2,%xmm2  
        vpsubq   %xmm14,%xmm3,%xmm3  
        vmovdqa  %xmm2,32(%r9)
        vmovdqa  %xmm3,48(%r9)
 
        vpaddq   %xmm8,%xmm4,%xmm12
        vpaddq   %xmm8,%xmm5,%xmm14
        vpand    %xmm9,%xmm12,%xmm12
        vpand    %xmm9,%xmm14,%xmm14 
        vpsrlq   %xmm10,%xmm12,%xmm13 
        vpsrlq   %xmm10,%xmm14,%xmm15
        vpsubq   %xmm13,%xmm12,%xmm12
        vpsubq   %xmm15,%xmm14,%xmm14
        vpand    %xmm11,%xmm12,%xmm12 
        vpand    %xmm11,%xmm14,%xmm14
        vpsubq   %xmm12,%xmm4,%xmm4
        vpsubq   %xmm14,%xmm5,%xmm5
        vmovdqa  %xmm4,64(%r9)
        vmovdqa  %xmm5,80(%r9)
 
        vpaddq   %xmm8,%xmm6,%xmm12
        vpaddq   %xmm8,%xmm7,%xmm14
        vpand    %xmm9,%xmm12,%xmm12
        vpand    %xmm9,%xmm14,%xmm14
        vpsrlq   %xmm10,%xmm12,%xmm13 
        vpsrlq   %xmm10,%xmm14,%xmm15
        vpsubq   %xmm13,%xmm12,%xmm12
        vpsubq   %xmm15,%xmm14,%xmm14
        vpand    %xmm11,%xmm12,%xmm12
        vpand    %xmm11,%xmm14,%xmm14
        vpsubq   %xmm12,%xmm6,%xmm6
        vpsubq   %xmm14,%xmm7,%xmm7
        vmovdqa  %xmm6,96(%r9)
        vmovdqa  %xmm7,112(%r9)

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
        vmovdqa  0(%rax,%rsi),%xmm0
        vmovdqa  16(%rax,%rsi),%xmm1
        vmovdqa  32(%rax,%rsi),%xmm2
        vmovdqa  48(%rax,%rsi),%xmm3
        vmovdqa  64(%rax,%rsi),%xmm4
        vmovdqa  80(%rax,%rsi),%xmm5
        vmovdqa  96(%rax,%rsi),%xmm6
        vmovdqa  112(%rax,%rsi),%xmm7
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
        vmovq        16(%rcx),%xmm8    /* mask     */
        vpinsrq      $1,16(%rcx),%xmm8,%xmm8   
        vmovq       8(%rcx),%xmm9      /* shift S  */
        vmovq        56(%rcx),%xmm10     /* 2^S % p  */
        vpinsrq      $1,56(%rcx),%xmm10,%xmm10
        vmovq        64(%rcx),%xmm11     /* bias     */
        vpinsrq      $1,64(%rcx),%xmm11,%xmm11   
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

        vmovdqa   0(%rdx),%xmm0   /* get cauldron of Cfmt */
        vmovdqa  16(%rdx),%xmm1
        vmovdqa  32(%rdx),%xmm2
        vmovdqa  48(%rdx),%xmm3
        vmovdqa  64(%rdx),%xmm4
        vmovdqa  80(%rdx),%xmm5
        vmovdqa  96(%rdx),%xmm6
        vmovdqa 112(%rdx),%xmm7

//  Start of primary loop
pcas2:
   
        movq    %rax,%r9      /* copy Afmt to add index */
        shrq    $4,%r9        /* top nybble to bottom   */
        andq    $0x780,%r9     /* the nybble            */
        movq    %rax,%r10     /* copy Afmt to sub index */
        andq    $0x780,%r10     /* get bottom nybble      */
        shrq    $8,%rax       /* next byte of Afmt      */ 
        vpaddq    0(%r8,%r9),%xmm0,%xmm0    /* add in cauldron */
        vpaddq   16(%r8,%r9),%xmm1,%xmm1
        vpaddq   32(%r8,%r9),%xmm2,%xmm2
        vpaddq   48(%r8,%r9),%xmm3,%xmm3
        vpaddq   64(%r8,%r9),%xmm4,%xmm4
        vpaddq   80(%r8,%r9),%xmm5,%xmm5
        vpaddq   96(%r8,%r9),%xmm6,%xmm6
        vpaddq  112(%r8,%r9),%xmm7,%xmm7
        vpsubq    0(%r8,%r10),%xmm0,%xmm0
        vpsubq   16(%r8,%r10),%xmm1,%xmm1
        vpsubq   32(%r8,%r10),%xmm2,%xmm2
        vpsubq   48(%r8,%r10),%xmm3,%xmm3
        vpsubq   64(%r8,%r10),%xmm4,%xmm4
        vpsubq   80(%r8,%r10),%xmm5,%xmm5
        vpsubq   96(%r8,%r10),%xmm6,%xmm6
        vpsubq  112(%r8,%r10),%xmm7,%xmm7
        addq    %r11,%r8                 /* move on to next slice */
        subq    $1,%rbx
        jne     pcas2
//  End of primary loop

        vpand    %xmm8,%xmm0,%xmm12
        vpand    %xmm8,%xmm1,%xmm13
        vpand    %xmm8,%xmm2,%xmm14
        vpand    %xmm8,%xmm3,%xmm15
        vpxor    %xmm12,%xmm0,%xmm0
        vpxor    %xmm13,%xmm1,%xmm1
        vpxor    %xmm14,%xmm2,%xmm2
        vpxor    %xmm15,%xmm3,%xmm3
        vpsrld   %xmm9,%xmm12,%xmm12
        vpsrld   %xmm9,%xmm13,%xmm13
        vpsrld   %xmm9,%xmm14,%xmm14
        vpsrld   %xmm9,%xmm15,%xmm15
        vpmulld  %xmm10,%xmm12,%xmm12
        vpmulld  %xmm10,%xmm13,%xmm13
        vpmulld  %xmm10,%xmm14,%xmm14
        vpmulld  %xmm10,%xmm15,%xmm15
        vpaddq   %xmm12,%xmm0,%xmm0
        vpaddq   %xmm13,%xmm1,%xmm1
        vpaddq   %xmm14,%xmm2,%xmm2
        vpaddq   %xmm15,%xmm3,%xmm3
        vpaddq   %xmm11,%xmm0,%xmm0
        vpaddq   %xmm11,%xmm1,%xmm1
        vpaddq   %xmm11,%xmm2,%xmm2
        vpaddq   %xmm11,%xmm3,%xmm3
        vmovdqa  %xmm0,0(%rdx)
        vmovdqa  %xmm1,16(%rdx)
        vmovdqa  %xmm2,32(%rdx)
        vmovdqa  %xmm3,48(%rdx)

        vpand    %xmm8,%xmm4,%xmm12
        vpand    %xmm8,%xmm5,%xmm13
        vpand    %xmm8,%xmm6,%xmm14
        vpand    %xmm8,%xmm7,%xmm15
        vpxor    %xmm12,%xmm4,%xmm4
        vpxor    %xmm13,%xmm5,%xmm5
        vpxor    %xmm14,%xmm6,%xmm6
        vpxor    %xmm15,%xmm7,%xmm7
        vpsrld   %xmm9,%xmm12,%xmm12
        vpsrld   %xmm9,%xmm13,%xmm13
        vpsrld   %xmm9,%xmm14,%xmm14
        vpsrld   %xmm9,%xmm15,%xmm15
        vpmulld  %xmm10,%xmm12,%xmm12
        vpmulld  %xmm10,%xmm13,%xmm13
        vpmulld  %xmm10,%xmm14,%xmm14
        vpmulld  %xmm10,%xmm15,%xmm15
        vpaddq   %xmm12,%xmm4,%xmm4
        vpaddq   %xmm13,%xmm5,%xmm5
        vpaddq   %xmm14,%xmm6,%xmm6
        vpaddq   %xmm15,%xmm7,%xmm7
        vpaddq   %xmm11,%xmm4,%xmm4
        vpaddq   %xmm11,%xmm5,%xmm5
        vpaddq   %xmm11,%xmm6,%xmm6
        vpaddq   %xmm11,%xmm7,%xmm7
        vmovdqa  %xmm4,64(%rdx)
        vmovdqa  %xmm5,80(%rdx)
        vmovdqa  %xmm6,96(%rdx)
        vmovdqa  %xmm7,112(%rdx)

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
        cmpq    $16,%rcx
        jb      pcxor5
        subq    $16,%rcx

pcxor1:
        vmovdqu  (%rsi,%rax),%xmm0
        vmovdqu  (%rdx,%rax),%xmm1
        vpxor    %xmm1,%xmm0,%xmm0
        vmovdqu  %xmm0,(%rdi,%rax)
        addq    $16,%rax
        cmpq    %rcx,%rax
        jbe     pcxor1

        addq    $16,%rcx
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

//  pcbif(d,s1,s2,nob,t)  a = t[(b*256)+c] (all nob bytes long)
//                         di  r8[(si*256)+dx]   cx
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
        pushq   %r15

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
        cmpq    $255,%rbx     /* have we finished yet   */
        je      getout2       /* yes - return           */
        movq    $1920,%r15
        notq    %r15
loop2:
        salq    $7, %rbx      /* 128 * byte Afmt        */
        addq    %rbx,%rdx     /* skip some rows of Cfmt */

        rorq    $1,%rcx
        andn    %rcx,%r15,%rax
        rorq    $4,%rcx
        andn    %rcx,%r15,%rbp
        rorq    $4,%rcx
        andn    %rcx,%r15,%r8
        rorq    $4,%rcx
        andn    %rcx,%r15,%rbx
        rorq    $4,%rcx
        andn    %rcx,%r15,%r9
        rorq    $4,%rcx
        andn    %rcx,%r15,%r10
        rorq    $4,%rcx
        andn    %rcx,%r15,%r11
        rorq    $4,%rcx
        andn    %rcx,%r15,%rcx

        addq    $5,%rdi       /* point to next Afmt word*/

	vmovdqa	0(%rdx), %xmm0 /* get 128 bytes of Cfmt */
	vmovdqa	16(%rdx), %xmm1
	vmovdqa	32(%rdx), %xmm2
	vmovdqa	48(%rdx), %xmm3
	vmovdqa	64(%rdx), %xmm4
	vmovdqa	80(%rdx), %xmm5
	vmovdqa	96(%rdx), %xmm6
	vmovdqa	112(%rdx), %xmm7

        vpxor    0(%rsi,%rax),%xmm0,%xmm0
        vpxor    16(%rsi,%rax),%xmm1,%xmm1
        vpxor    32(%rsi,%rax),%xmm2,%xmm2
        vpxor    48(%rsi,%rax),%xmm3,%xmm3
        vpxor    64(%rsi,%rax),%xmm4,%xmm4
        vpxor    80(%rsi,%rax),%xmm5,%xmm5
        vpxor    96(%rsi,%rax),%xmm6,%xmm6
        vpxor    112(%rsi,%rax),%xmm7,%xmm7

        vpxor    2048(%rsi,%rbp),%xmm0,%xmm0
        vpxor    2064(%rsi,%rbp),%xmm1,%xmm1
        vpxor    2080(%rsi,%rbp),%xmm2,%xmm2
        vpxor    2096(%rsi,%rbp),%xmm3,%xmm3
        vpxor    2112(%rsi,%rbp),%xmm4,%xmm4
        vpxor    2128(%rsi,%rbp),%xmm5,%xmm5
        vpxor    2144(%rsi,%rbp),%xmm6,%xmm6
        vpxor    2160(%rsi,%rbp),%xmm7,%xmm7

        vpxor    4096(%rsi,%r8),%xmm0,%xmm0
        vpxor    4112(%rsi,%r8),%xmm1,%xmm1
        vpxor    4128(%rsi,%r8),%xmm2,%xmm2
        vpxor    4144(%rsi,%r8),%xmm3,%xmm3
        vpxor    4160(%rsi,%r8),%xmm4,%xmm4
        vpxor    4176(%rsi,%r8),%xmm5,%xmm5
        vpxor    4192(%rsi,%r8),%xmm6,%xmm6
        vpxor    4208(%rsi,%r8),%xmm7,%xmm7

        vpxor    6144(%rsi,%rbx),%xmm0,%xmm0
        vpxor    6160(%rsi,%rbx),%xmm1,%xmm1
        vpxor    6176(%rsi,%rbx),%xmm2,%xmm2
        vpxor    6192(%rsi,%rbx),%xmm3,%xmm3
        vpxor    6208(%rsi,%rbx),%xmm4,%xmm4
        vpxor    6224(%rsi,%rbx),%xmm5,%xmm5
        vpxor    6240(%rsi,%rbx),%xmm6,%xmm6
        vpxor    6256(%rsi,%rbx),%xmm7,%xmm7

        vpxor    8192(%rsi,%r9),%xmm0,%xmm0
        vpxor    8208(%rsi,%r9),%xmm1,%xmm1
        vpxor    8224(%rsi,%r9),%xmm2,%xmm2
        vpxor    8240(%rsi,%r9),%xmm3,%xmm3
        vpxor    8256(%rsi,%r9),%xmm4,%xmm4
        vpxor    8272(%rsi,%r9),%xmm5,%xmm5
        vpxor    8288(%rsi,%r9),%xmm6,%xmm6
        vpxor    8304(%rsi,%r9),%xmm7,%xmm7

        vpxor    10240(%rsi,%r10),%xmm0,%xmm0
        vpxor    10256(%rsi,%r10),%xmm1,%xmm1
        vpxor    10272(%rsi,%r10),%xmm2,%xmm2
        vpxor    10288(%rsi,%r10),%xmm3,%xmm3
        vpxor    10304(%rsi,%r10),%xmm4,%xmm4
        vpxor    10320(%rsi,%r10),%xmm5,%xmm5
        vpxor    10336(%rsi,%r10),%xmm6,%xmm6
        vpxor    10352(%rsi,%r10),%xmm7,%xmm7

        vpxor    12288(%rsi,%r11),%xmm0,%xmm0
        vpxor    12304(%rsi,%r11),%xmm1,%xmm1
        vpxor    12320(%rsi,%r11),%xmm2,%xmm2
        vpxor    12336(%rsi,%r11),%xmm3,%xmm3
        vpxor    12352(%rsi,%r11),%xmm4,%xmm4
        vpxor    12368(%rsi,%r11),%xmm5,%xmm5
        vpxor    12384(%rsi,%r11),%xmm6,%xmm6
        vpxor    12400(%rsi,%r11),%xmm7,%xmm7

        vpxor    14336(%rsi,%rcx),%xmm0,%xmm0
        vpxor    14352(%rsi,%rcx),%xmm1,%xmm1
        vpxor    14368(%rsi,%rcx),%xmm2,%xmm2
        vpxor    14384(%rsi,%rcx),%xmm3,%xmm3
        vpxor    14400(%rsi,%rcx),%xmm4,%xmm4
        vpxor    14416(%rsi,%rcx),%xmm5,%xmm5
        vpxor    14432(%rsi,%rcx),%xmm6,%xmm6
        vpxor    14448(%rsi,%rcx),%xmm7,%xmm7

        vmovdqa  %xmm0,0(%rdx)
        vmovdqa  %xmm1,16(%rdx)
        vmovdqa  %xmm2,32(%rdx)
        vmovdqa  %xmm3,48(%rdx)
        vmovdqa  %xmm4,64(%rdx)
        vmovdqa  %xmm5,80(%rdx)
        vmovdqa  %xmm6,96(%rdx)
        vmovdqa  %xmm7,112(%rdx)

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
        jne     loop2
getout2:
        popq    %r15
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
	vmovdqa	(%rdi),%xmm0
	vmovdqa	64(%rdi),%xmm1
        vmovdqa  (%rsi),%xmm2
        vmovdqa  64(%rsi),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  %xmm3,(%rdx)
        vmovdqa  %xmm2,64(%rdx)

	vmovdqa	16(%rdi),%xmm0
	vmovdqa	80(%rdi),%xmm1
        vmovdqa  16(%rsi),%xmm2
        vmovdqa  80(%rsi),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  %xmm3,16(%rdx)
        vmovdqa  %xmm2,80(%rdx)

	vmovdqa	32(%rdi),%xmm0
	vmovdqa	96(%rdi),%xmm1
        vmovdqa  32(%rsi),%xmm2
        vmovdqa  96(%rsi),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  %xmm3,32(%rdx)
        vmovdqa  %xmm2,96(%rdx)

	vmovdqa	48(%rdi),%xmm0
	vmovdqa	112(%rdi),%xmm1
        vmovdqa  48(%rsi),%xmm2
        vmovdqa  112(%rsi),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  %xmm3,48(%rdx)
        vmovdqa  %xmm2,112(%rdx)
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

	vmovdqa	(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	vmovdqa	64(%rdx),%xmm1
        vmovdqa  (%rsi,%rax),%xmm2
        vmovdqa  (%rsi,%rbx),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  5248(%rsi,%rbp),%xmm0
        vmovdqa  5248(%rsi,%r9),%xmm1
        vpxor    %xmm3,%xmm0,%xmm0
        vpxor    %xmm1,%xmm2,%xmm2
        vpxor    %xmm0,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpor     %xmm1,%xmm2,%xmm2
        vpor     %xmm0,%xmm3,%xmm3
        vmovdqa  10496(%rsi,%rcx),%xmm1
        vmovdqa  10496(%rsi,%r10),%xmm0
        vpxor    %xmm2,%xmm1,%xmm1
        vpxor    %xmm0,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpxor    %xmm3,%xmm2,%xmm2
        vpor     %xmm3,%xmm0,%xmm0
        vpor     %xmm2,%xmm1,%xmm1
        vmovdqa  %xmm0,(%rdx)
        vmovdqa  %xmm1,64(%rdx)

	vmovdqa	16(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	vmovdqa	80(%rdx),%xmm1
        vmovdqa  16(%rsi,%rax),%xmm2
        vmovdqa  16(%rsi,%rbx),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  5264(%rsi,%rbp),%xmm0
        vmovdqa  5264(%rsi,%r9),%xmm1
        vpxor    %xmm3,%xmm0,%xmm0
        vpxor    %xmm1,%xmm2,%xmm2
        vpxor    %xmm0,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpor     %xmm1,%xmm2,%xmm2
        vpor     %xmm0,%xmm3,%xmm3
        vmovdqa  10512(%rsi,%rcx),%xmm1
        vmovdqa  10512(%rsi,%r10),%xmm0
        vpxor    %xmm2,%xmm1,%xmm1
        vpxor    %xmm0,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpxor    %xmm3,%xmm2,%xmm2
        vpor     %xmm3,%xmm0,%xmm0
        vpor     %xmm2,%xmm1,%xmm1
        vmovdqa  %xmm0,16(%rdx)
        vmovdqa  %xmm1,80(%rdx)

	vmovdqa	32(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	vmovdqa	96(%rdx),%xmm1
        vmovdqa  32(%rsi,%rax),%xmm2
        vmovdqa  32(%rsi,%rbx),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  5280(%rsi,%rbp),%xmm0
        vmovdqa  5280(%rsi,%r9),%xmm1
        vpxor    %xmm3,%xmm0,%xmm0
        vpxor    %xmm1,%xmm2,%xmm2
        vpxor    %xmm0,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpor     %xmm1,%xmm2,%xmm2
        vpor     %xmm0,%xmm3,%xmm3
        vmovdqa  10528(%rsi,%rcx),%xmm1
        vmovdqa  10528(%rsi,%r10),%xmm0
        vpxor    %xmm2,%xmm1,%xmm1
        vpxor    %xmm0,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpxor    %xmm3,%xmm2,%xmm2
        vpor     %xmm3,%xmm0,%xmm0
        vpor     %xmm2,%xmm1,%xmm1
        vmovdqa  %xmm0,32(%rdx)
        vmovdqa  %xmm1,96(%rdx)

	vmovdqa	48(%rdx),%xmm0   /* get 32 bytes of Cfmt */
	vmovdqa	112(%rdx),%xmm1
        vmovdqa  48(%rsi,%rax),%xmm2
        vmovdqa  48(%rsi,%rbx),%xmm3
        vpxor    %xmm0,%xmm2,%xmm2
        vpxor    %xmm3,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpor     %xmm1,%xmm3,%xmm3
        vpor     %xmm0,%xmm2,%xmm2
        vmovdqa  5296(%rsi,%rbp),%xmm0
        vmovdqa  5296(%rsi,%r9),%xmm1
        vpxor    %xmm3,%xmm0,%xmm0
        vpxor    %xmm1,%xmm2,%xmm2
        vpxor    %xmm0,%xmm1,%xmm1
        vpxor    %xmm2,%xmm3,%xmm3
        vpor     %xmm1,%xmm2,%xmm2
        vpor     %xmm0,%xmm3,%xmm3
        vmovdqa  10544(%rsi,%rcx),%xmm1
        vmovdqa  10544(%rsi,%r10),%xmm0
        vpxor    %xmm2,%xmm1,%xmm1
        vpxor    %xmm0,%xmm3,%xmm3
        vpxor    %xmm1,%xmm0,%xmm0
        vpxor    %xmm3,%xmm2,%xmm2
        vpor     %xmm3,%xmm0,%xmm0
        vpor     %xmm2,%xmm1,%xmm1
        vmovdqa  %xmm0,48(%rdx)
        vmovdqa  %xmm1,112(%rdx)

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
