/* pcrit1s.s assembler for SSE version */
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

// mactype (char res[8])
//             rdi 

	.text
	.globl	mactype
	.type	mactype, @function
mactype:
        pushq   %rbx
        movb    $0x61,%r8b    /* Class starts off as 'a'  */

/*  check lahf-lm, cx16 and sse3 to move to class 'b'     */
        movl    $0x80000001,%eax
        cpuid

        testl   $1,%ecx       /* lahf-lm set? */
        je      macp2         /* if not, class is 'a' */
        movl    $1,%eax
        cpuid
        movl    %ecx,%r9d     /* save ecx for later in r9 */
        movl    %eax,%r10d    /* family stuff for later */
        testl   $0x2000,%ecx  /* cx16 bit set?  */
        je      macp2         /* if not, class is 'a' */
        testl   $1,%ecx       /* SSE3 - pni bit set?  */
        je      macp2         /* if not, class is 'a' */
        movb    $0x62,%r8b    /* Class at least 'b'  */

/*  check SSSE3 to move to class 'c'  */
        testl   $0x200,%ecx   /* SSSE3 - bit set?  */
        je      macp2         /* if not, class is 'b' */
        movb    $0x63,%r8b    /* Class at least 'c'  */
/*  check SSE4.1 to move to class 'd'  */
        testl   $0x80000,%ecx   /* SSE4.1 - bit set?  */
        je      macp2         /* if not, class is 'c' */
        movb    $0x64,%r8b    /* Class at least 'd'  */
/*  check SSE4.2 to move to class 'e'  */
        testl   $0x100000,%ecx   /* SSE4.2 - bit set?  */
        je      macp2         /* if not, class is 'd' */
        movb    $0x65,%r8b    /* Class at least 'e'  */
/*  check CLMUL to move to class 'f'  */
        testl   $0x2,%ecx     /* CLMUL - bit set?  */
        je      macp2         /* if not, class is 'e' */
        movb    $0x66,%r8b    /* Class at least 'f'  */
/*  check AVX-1 to move to class 'g'  */
        testl   $0x10000000,%ecx     /* AVX - bit set?  */
        je      macp2         /* if not, class is 'e' */
        movb    $0x67,%r8b    /* Class at least 'g'  */
/*  check BMI1 to move to class 'h'  */
        movl    $0x7,%eax
        movl    $0,%ecx
        cpuid
        movl    %r10d,4(%rdi)
        testl   $8,%ebx       /* check bmi1 bit */
        je      macp2         /* not set - class is 'g' */
        movb    $0x68,%r8b    /* else class is at least 'h' */
/*  check BMI2 to move to class 'i'  */
        testl   $0x100,%ebx     /* BMI2 - bit set?  */
        je      macp2         /* if not, class is 'h' */
        movb    $0x69,%r8b    /* Class at least 'i'  */
/*  check AVX2 and MOVBE to move to class 'j'  */
        testl   $0x20,%ebx     /* AVX2 - bit set?  */
        je      macp2         /* if not, class is 'i' */
        testl   $0x400000,%r9d /* test MOVBE bit */
        je      macp2         /* class j needs MOVBE as well */
        movb    $0x6A,%r8b    /* Class at least 'j'  */

macp2:
        movb    %r8b,(%rdi)   /* put class into mact[0]   */
        movl    %r10d,%eax
        shrl    $20,%eax
        shrl    $8,%r10d
        andl    $15,%r10d
        andl    $255,%eax
        addl    %r10d,%eax    /* so eax is the family number */
        movb    $0x32,1(%rdi) /* put cache indicator = '2' */
        cmpl    $0x15,%eax    /* for Bulldozer-Steamroller family */
        je      macp4
        movb    $0x30,1(%rdi) /* else cache indicator = '0' */
macp4:
        popq    %rbx
        ret
	.size	mactype, .-mactype


// pccl32 parms scalar noc  d1  d2
//         rdi   rsi   rdx rcx  r8

	.text
	.globl	pccl32
	.type	pccl32, @function
// initialization
pccl32:
       testq    %rdx,%rdx
       je       pccl32ret
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
pccl32ret:
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
       testq    %rdx,%rdx
       je       pccl64ret
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
pccl64ret:
        ret
	.size	pccl64, .-pccl64

// end of pccl64


// pcdasc rdi=prog rsi=bwa rdx=parms
// rax=program byte, r8=prog ptr  r9=place to store
// r10 slices left rcx slice stride

// unused r11
// untouchted rbp r12 r13 r14 r15

	.text
	.globl	pcdasc
	.type	pcdasc, @function
pcdasc:                   /* initialization */
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
pcdasc1:                   /* next slice */
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
pcdasc2:
        movzbq  0(%r8),%rax   /* get first/next program byte */
        addq    $1,%r8        /* move on to next program byte */
        cmpq    $79,%rax      /* is it a star-move */
        ja      pcdasc4       /* no - go see what it is */
pcdasc3:
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
        jbe     pcdasc3      /* yes - go straight round again */
pcdasc4:
        cmpq    $159,%rax
        ja      pcdasc5      /* not a load of accumulator either */
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
        jmp     pcdasc2
pcdasc5:
        cmpq    $239,%rax
        ja      pcdasc6       /* not a set destination either */
        subq    $160,%rax
        shlq    $7,%rax        /* multiply by slot size */
        movq    %rax,%r9
        addq    %rsi,%r9
        jmp     pcdasc2
pcdasc6:                      /* anything 240+ is stop at the moment */
        addq    %rcx,%rsi      /* add in slice stride */
        subq    $1,%r10        /* subtract 1 from slice count */
        jne     pcdasc1
        ret

	.size	pcdasc, .-pcdasc

// end of pcdasc

// pcaas Afmt bwa Cfmt parms

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pcaas
	.type	pcaas, @function
pcaas:
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
        je      pcaas8      /* yes get straight out */
//  Start of secondary loop
pcaas1:
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
pcaas2:
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
        jne     pcaas2
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
        jne     pcaas1         /* no - round again     */
//  End of secondary loop
pcaas8:
        popq    %rbx
        ret      
	.size	pcaas, .-pcaas

// end of pcaas

// pcdas Afmt bwa Cfmt parms

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pcdas
	.type	pcdas, @function
pcdas:
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
        je      pcdas8      /* yes get straight out */
//  Start of secondary loop
pcdas1:
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
pcdas2:
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
        jne     pcdas2
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
        jne     pcdas1         /* no - round again     */
//  End of secondary loop
pcdas8:
        popq    %rbx
        ret      
	.size	pcdas, .-pcdas

// end of pcdas


// pcbmas Afmt bwa Cfmt parms

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pcjas
	.type	pcjas, @function
pcjas:
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
        je      pcjas5        /* yes get straight out */
//  Start of secondary loop
pcjas1:
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
pcjas2:
   
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
        jne     pcjas2
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
        jne     pcjas1         /* no - round again     */
//  End of secondary loop
pcjas5:
        popq    %rbx
        ret      
	.size	pcjas, .-pcjas

// end of pcjas

// pcjat Afmt bwa Cfmt parms

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pcjat
	.type	pcjat, @function
pcjat:
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
        je      pcjat5        /* yes get straight out */
//  Start of secondary loop
pcjat1:
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
pcjat2:
   
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
        jne     pcjat2
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
        jne     pcjat1         /* no - round again     */
//  End of secondary loop
pcjat5:
        popq    %rbx
        ret      
	.size	pcjat, .-pcjat

// end of pcjat


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

//  pcaxor(d.s1,s2,nob)  d = s1^s2 (all nob bytes long)
//                      rdi rsi rdx     rcx

	.text
	.globl	pcaxor
	.type	pcaxor, @function
pcaxor:
        xorq    %rax,%rax
        cmpq    $16,%rcx
        jb      pcaxor5
        subq    $16,%rcx

pcaxor1:
        movdqu  (%rsi,%rax),%xmm0
        movdqu  (%rdx,%rax),%xmm1
        pxor    %xmm1,%xmm0
        movdqu  %xmm0,(%rdi,%rax)
        addq    $16,%rax
        cmpq    %rcx,%rax
        jbe     pcaxor1

        addq    $16,%rcx
pcaxor5:
        cmpq    %rcx,%rax
        je      pcaxor9
pcaxor7:
        movb    (%rsi,%rax),%r8b
        xorb    (%rdx,%rax),%r8b
        movb    %r8b,(%rdi,%rax)
        addq    $1,%rax
        cmpq    %rcx,%rax
        jb      pcaxor7

pcaxor9:
        ret      
	.size	pcaxor, .-pcaxor

//  pcjxor(d.s1,s2,nob)  d = s1^s2 (all nob bytes long)
//                      rdi rsi rdx     rcx

	.text
	.globl	pcjxor
	.type	pcjxor, @function
pcjxor:
        xorq    %rax,%rax
        cmpq    $32,%rcx
        jb      pcjxor5
        subq    $32,%rcx

pcjxor1:
        vmovdqu  (%rsi,%rax),%ymm0
        vmovdqu  (%rdx,%rax),%ymm1
        vpxor    %ymm1,%ymm0,%ymm0
        vmovdqu  %ymm0,(%rdi,%rax)
        addq    $32,%rax
        cmpq    %rcx,%rax
        jbe     pcjxor1

        addq    $32,%rcx
pcjxor5:
        cmpq    %rcx,%rax
        je      pcjxor9
pcjxor7:
        movb    (%rsi,%rax),%r8b
        xorb    (%rdx,%rax),%r8b
        movb    %r8b,(%rdi,%rax)
        addq    $1,%rax
        cmpq    %rcx,%rax
        jb      pcjxor7

pcjxor9:
        ret      
	.size	pcjxor, .-pcjxor


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
	.globl	pcab2
	.type	pcab2, @function
pcab2:
        pushq   %rbx
        pushq   %rbp

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pcab25        /* yes - return           */
pcab22:
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
        jne     pcab22
pcab25:
        popq    %rbp
        popq    %rbx
  
        ret      
	.size	pcab2, .-pcab2

/* rdi Afmt     rsi bwa     rdx  Cfmt  */
/* rcx count  rax/r8 bwa addresses r9 bwa ptr*/
	.text
	.globl	pcjb2
	.type	pcjb2, @function
pcjb2:
        pushq   %rbx
        pushq   %rbp

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pcjb25        /* yes - return           */
pcjb21:
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
        jne     pcjb21
pcjb25:
        popq    %rbp
        popq    %rbx
  
        ret      
	.size	pcjb2, .-pcjb2

/* pcad3 a, b, c.  c=a+b  */
/* rdi input1   rsi input2  rdx output  */

	.text
	.globl	pcad3
	.type	pcad3, @function
pcad3:
	movdqa	(%rdi),%xmm0
	movdqa	64(%rdi),%xmm1
        movdqa  (%rsi),%xmm2
        movdqa  64(%rsi),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  %xmm3,(%rdx)
        movdqa  %xmm2,64(%rdx)

	movdqa	16(%rdi),%xmm0
	movdqa	80(%rdi),%xmm1
        movdqa  16(%rsi),%xmm2
        movdqa  80(%rsi),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  %xmm3,16(%rdx)
        movdqa  %xmm2,80(%rdx)

	movdqa	32(%rdi),%xmm0
	movdqa	96(%rdi),%xmm1
        movdqa  32(%rsi),%xmm2
        movdqa  96(%rsi),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  %xmm3,32(%rdx)
        movdqa  %xmm2,96(%rdx)

	movdqa	48(%rdi),%xmm0
	movdqa	112(%rdi),%xmm1
        movdqa  48(%rsi),%xmm2
        movdqa  112(%rsi),%xmm3
        pxor    %xmm0,%xmm2
        pxor    %xmm3,%xmm1
        pxor    %xmm2,%xmm3
        pxor    %xmm1,%xmm0
        por     %xmm1,%xmm3
        por     %xmm0,%xmm2
        movdqa  %xmm3,48(%rdx)
        movdqa  %xmm2,112(%rdx)
        ret      
	.size	pcab3, .-pcad3

/* input  rdi Afmt     rsi bwa     rdx  Cfmt  */

/* six registers point in bwa  0   rax   rbx  */
/*                             1   rbp   r9   */
/*                             2   rcx   r10  */
	.text
	.globl	pcab3
	.type	pcab3, @function
pcab3:
        pushq   %rbx
        pushq   %rbp
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pcab35        /* yes - return           */
pcab32:
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
        jne     pcab32
pcab35:
        popq    %rbp
        popq    %rbx
        ret      
	.size	pcab3, .-pcab3

/* input  rdi Afmt     rsi bwa     rdx  Cfmt  */

/* six registers point in bwa  0   rax   rbx  */
/*                             1   rbp   r9   */
/*                             2   rcx   r10  */
	.text
	.globl	pcjb3
	.type	pcjb3, @function
pcjb3:
        pushq   %rbx
        pushq   %rbp
        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      pcjb35        /* yes - return           */
pcjb33:
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
        jne     pcjb33
pcjb35:
        popq    %rbp
        popq    %rbx
        ret      
	.size	pcjb3, .-pcjb3

