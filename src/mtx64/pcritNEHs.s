/* pcrit1s.s assembler for SSE version */
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
        movdqu  (%rsi,%rax),%xmm0
        movdqu  (%rdx,%rax),%xmm1
        pxor    %xmm1,%xmm0
        movdqu  %xmm0,(%rdi,%rax)
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

        movq    (%rdi),%rcx   /* get Afmt word  */
        movq    %rcx,%rbx     /* copy for skip  */
        andq    $255,%rbx
	cmpq	$255,%rbx     /* have we finished yet   */
	je      getout2       /* yes - return           */
loop2:
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
        jne     loop2
getout2:
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
        jne     loop3
getout3:
        popq    %rbp
        popq    %rbx
        ret      
	.size	pcbm3, .-pcbm3
