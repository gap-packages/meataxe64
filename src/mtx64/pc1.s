/* pc1.s x86 assembler non-HPMI functions*/
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

// mactype (char res[8])
//             rdi 

	.text
	.globl	mactype
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
        je      macp1         /* if not, class is 'h' or 'k' */
        movb    $0x69,%r8b    /* Class at least 'i'  */
/*  check AVX2 and MOVBE to move to class 'j'  */
        testl   $0x20,%ebx     /* AVX2 - bit set?  */
        je      macp1         /* if not, check FMA */
        testl   $0x400000,%r9d /* test MOVBE bit */
        je      macp1         /* class j needs MOVBE as well */
        movb    $0x6A,%r8b    /* Class at least 'j'  */
        testl   $0x1000,%r9d /* test FMA bit */
        je      macp2        /* no fma, class 'j' */
        movb    $0x6C,%r8b   /* class 'l' if all of them */
        testl   $0x10000,%ebx  /* AVX512 available? */
        je      macp2        /* if not, class 'l' */
        movb    $0x6D,%r8b   /* class 'm' if it is there */  
/* future tests in advance of 'm' go here */
        jmp     macp2   
macp1:                       /* no BMI2/AVX2/MOVBE but FMA? */
        testl   $0x1000,%r9d /* test FMA bit */
        je      macp2        /* no fma, class 'i' */
        movb    $0x6B,%r8b   /* class 'k' if fma but not avx2/movbe */

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
        movb    $0x31,1(%rdi) /* put cache indicator = '1' */
        cmpl    $0x17,%eax    /* for Ryzen family */
        je      macp4
        movb    $0x30,1(%rdi) /* else cache indicator = '0' */
macp4:
        popq    %rbx
        ret


// pccl32 parms scalar noc  d1  d2
//         rdi   rsi   rdx rcx  r8

	.text
	.globl	pccl32
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

// end of pccl32

// pccl64 parms scalar noc  d1  d2
//         rdi   rsi   rdx rcx  r8

	.text
	.globl	pccl64
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

// end of pccl64

// void pcbunf(Dfmt * d, const Dfmt * s, uint64_t nob,
//               %rdi            %rsi         %rdx
//                   const uint8_t * t1, const uint8_t * t2)
//                         %rcx              %r8
	.text
	.globl	pcbunf
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

// void pcxunf(Dfmt * d, const Dfmt * s, uint64_t nob,
//               %rdi            %rsi         %rdx
//                   const uint8_t * t1)
//                         %rcx
	.text
	.globl	pcxunf
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


// void pcunf(Dfmt * d, uint64_t nob, const uint8_t * t1)
//              %rdi          %rsi         %rdx

	.text
	.globl	pcunf
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


// uint64_t pcpmad(uint64_t p,uint64_t a,uint64_t b,uint64_t c);
//  %rax             %rdi         %rsi       %rdx       %rcx
	.text
	.globl	pcpmad
pcpmad:
        movq    %rdx,%rax
        mulq    %rsi
        addq    %rcx,%rax
        adcq    $0,%rdx
        divq    %rdi      /* %rax quot,  %rdx rem  */
        movq    %rdx,%rax
        ret      

// uint64_t pcrem(uint64_t p,uint64_t xl,uint64_t xh);
//  %rax             %rdi         %rsi       %rdx 
	.text
	.globl	pcrem
pcrem:
        movq    %rsi,%rax
        divq    %rdi      /* %rax quot,  %rdx rem  */
        movq    %rdx,%rax
        ret  

//  pc1xora(d.s1,s2,nob)  d = s1^s2 (all nob bytes long)
//                      rdi rsi rdx     rcx

	.text
	.globl	pc1xora
pc1xora:
        xorq    %rax,%rax
        cmpq    $16,%rcx
        jb      pc1xora5
        subq    $16,%rcx

pc1xora1:
        movdqu  (%rsi,%rax),%xmm0
        movdqu  (%rdx,%rax),%xmm1
        pxor    %xmm1,%xmm0
        movdqu  %xmm0,(%rdi,%rax)
        addq    $16,%rax
        cmpq    %rcx,%rax
        jbe     pc1xora1

        addq    $16,%rcx
pc1xora5:
        cmpq    %rcx,%rax
        je      pc1xora9
pc1xora7:
        movb    (%rsi,%rax),%r8b
        xorb    (%rdx,%rax),%r8b
        movb    %r8b,(%rdi,%rax)
        addq    $1,%rax
        cmpq    %rcx,%rax
        jb      pc1xora7

pc1xora9:
        ret      

//  pc1xorj(d.s1,s2,nob)  d = s1^s2 (all nob bytes long)
//                      rdi rsi rdx     rcx

	.text
	.globl	pc1xorj
pc1xorj:
        xorq    %rax,%rax
        cmpq    $32,%rcx
        jb      pc1xorj5
        subq    $32,%rcx

pc1xorj1:
        vmovdqu  (%rsi,%rax),%ymm0
        vmovdqu  (%rdx,%rax),%ymm1
        vpxor    %ymm1,%ymm0,%ymm0
        vmovdqu  %ymm0,(%rdi,%rax)
        addq    $32,%rax
        cmpq    %rcx,%rax
        jbe     pc1xorj1

        addq    $32,%rcx
pc1xorj5:
        cmpq    %rcx,%rax
        je      pc1xorj9
pc1xorj7:
        movb    (%rsi,%rax),%r8b
        xorb    (%rdx,%rax),%r8b
        movb    %r8b,(%rdi,%rax)
        addq    $1,%rax
        cmpq    %rcx,%rax
        jb      pc1xorj7

pc1xorj9:
        ret      


//  pcbif(d,s1,s2,nob,t)  a = t[(b*256)+c] (all nob bytes long)
//                         di  r8[(si*256)+dx]   cx
	.text
	.globl	pcbif
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

// pcbarprp inp, oup, base, digits, maxval, &barpar
//          rdi  rsi  rdx    rcx      r8      r9

	.text
	.globl	pcbarprp
pcbarprp:
        movq    %rdx,8(%r9)   /*  base   */
        subq    $1,%rcx       /* one less than actual digits */
        movq    %rcx,16(%r9)  /*  digits */
        movq    $1,%rcx       /* number of bits in base */
pcbarp1:
        shrq    $1,%rdx
        jz      pcbarp2
        addq    $1,%rcx
        jmp     pcbarp1
pcbarp2:                      /* %rcx is number of bits to shift */
        subq    $1,%rcx
        movq    %rcx,24(%r9)
        movq    $1,%rdx
        shlq    %cl,%rdx
        xorq    %rax,%rax
        divq    8(%r9)         /* rax quot, rdx rem */
        addq    $1,%rax        /* round up */
        movq    %rax,32(%r9)
        movq    $0,%rax        /* start flags at zero */
        movq    %r8,%rdx
        shrq    $63,%rdx
        jz      pcbarp7        /* OK if max to bit not set */
        addq    $16,%rax       /* else first round is divide */
pcbarp7:
        cmpq    $1,%rdi
        je      pcbarp8
        addq    $4,%rax
        cmpq    $2,%rdi
        je      pcbarp8
        addq    $4,%rax
pcbarp8:
        cmpq    $2,%rsi
        ja      pcbarp9
        addq    $1,%rax
        cmpq    $1,%rsi
        ja      pcbarp9
        addq    $1,%rax
pcbarp9:
        movq    %rax,(%r9)    /*  flags */
        ret
// end of pcbarprp

// barpar flags base digits shift multip ...
//        0()   8()   16()   24()   32()
// flags byte  0000xxyy  (xx 00=32 01=16 10=64)  (yy 00=8 01=16 10=32)

//pcbarrett &barpar, &input, &output, entries  stride
//            rdi      rsi     rdx      rcx     %r8

	.text
	.globl	pcbarrett
pcbarrett:          /* first set up the registers */
        push   %rbx
        push   %r14
        push   %r15
        movq   %rcx,%r10    /* get outer loop count out of the way */
        movq   %rdx,%r14    /* get output pointer out of the way  */
        movb   0(%rdi),%ch  /* flags  */
        movq   8(%rdi),%r9  /* base = denominator  */
        movb   16(%rdi),%bh /* (constant) number of digits  */
        movb   24(%rdi),%cl /* shift */
        movq   32(%rdi),%r15 /* Barrett multiplier */
// now we've finished with %rdi as parameter pointer

// Register usage   rax rdx rdi computation registers - see below
// ch  flags    cl shift
// bh  digits   bl digits decreasing
// rsi   input pointer
// rdi   parameters then X for arithmetic
// %r15   multiplier
// %r8   stride
// %r9   base = denominator
// %r10  outer loop count
// %r11  output pointer
// %r14  first matrix output pointer

pcbarb1:
        movb   %bh,%bl      /* copy of digits  */
        movq   %r14,%r11    /* get output pointer */
        testb  $12,%ch      /* 32-bit input?  */
        jne    pcbarb11     /* no - sort it out  */
        movl   0(%rsi),%eax /* 32-bit load with zero extension  */
        addq   $4,%rsi      /* next 32-bit number  */
pcbarb2:
        testb  $16,%ch      /* do we do first digit by division */
        jne    pcbard2      /* yes, so do the division */
pcbarm2:                    /* X is in %rax at this point */
        movq   %rax,%rdi    /* save a copy of X in %rdi */
        mul    %r15          /* 64 -> 128 multiply */
        shr    %cl,%rdx     /* complete the Barrett, %rdx=q */
        movq   %rdx,%rax    /* keep a copy of q ready for next round */
        imul   %r9,%rdx     /* rdx = q.d  */
        subq   %rdx,%rdi    /* so rdi is now remainder */
pcbarm3:     /* rdi remainder  rax quotient */
        testb  $3,%ch       /* 8 bit output  */
        jne    pcbarm31     /* no - sort it out  */
        movb   %dil,0(%r11) /* 8 bit store  */
pcbarm4:
        addq   %r8,%r11
        subb   $1,%bl       /* decrement digit count  */
        jne    pcbarm2      /* go do more digits  */
        testb  $3,%ch       /* last digit */
        jne    pcbarm41
        movb   %al,0(%r11)
        addq   $1,%r14
pcbarm5:
        subq   $1,%r10      /* decrement input words  */
        jne    pcbarb1      /* more words to do  */
        pop    %r15
        pop    %r14
        pop    %rbx
        ret

pcbarb11:         /* input is not 32 bits */
        testb  $4,%ch       /* 64-bit load?  */
        jne    pcbarb12     /* no - sort that out  */
        movq   0(%rsi),%rax /* 64-bit load  */
        addq   $8,%rsi      /* next 64-bit number  */
        jmp    pcbarb2
pcbarb12:
        movzwq 0(%rsi),%rax  /* 16 bit load with zero extension  */
        addq   $2,%rsi       /* next 16 bit number  */
        jmp    pcbarb2

pcbarm31:            /* store is not 8 bits */
        testb  $2,%ch        /* 16 bit store?  */
        jne    pcbarm32
        movw   %di,0(%r11)   /* 16 bit store  */
        jmp    pcbarm4
pcbarm32:
        movl   %edi,0(%r11)  /* 32-bit store  */
        jmp    pcbarm4

pcbarm41:            /* last store, not 8 bits */
        testb  $2,%ch        /* 16 bit store?  */
        jne    pcbarm42
        movw   %ax,0(%r11)   /* 16 bit store  */
        addq   $2,%r14       /* 16 bit increment  */
        jmp    pcbarm5
pcbarm42:
        movl   %eax,0(%r11)  /* 32-bit store  */
        addq   $4,%r14       /* 32-bit increment  */
        jmp    pcbarm5

pcbard2:
        xorq   %rdx,%rdx    /* clear rdx ready for divide  */
        div    %r9          /* rax quot,  rdx rem  */
        movq   %rdx,%rdi    /* put remainder into %rdi */
        jmp pcbarm3

// end of pcbarrett

//  end of pc1.s
