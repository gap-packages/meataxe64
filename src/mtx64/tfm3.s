/* tfm3.s V3 thread-farm primitives  */
/* R. A. Parker 2.12.2019       */

/* parms rdi rsi rdx rcx r8 r9 */
/* scratch rax (RV) r10 r11 */
/* not scratch rbx rbp r12 r13 r14 r15 */

/* tfm3bup ( heap heapsize bitstring jobs freethis ) */
/*           %rdi   %rsi     %rdx    %rcx    %r8 */

/* non-crical code shifted to emphasise  */

/* register usage throughout */
/*  %al byte variables (lock, batch type)  %ah second byte variable */
/*  %rdi start of heap  %r10 displacement in heap %rsi biggest such */
/*  %rcx output jobs area */

	.text
	.globl	tfm3bup
tfm3bup:
/*  do as much as possible before entering critical region */
        pushq   %rbp
        pushq   %rbx
        pushq   %r14
        pushq   %r15
        movq    $64,%r10   /* set T=1 in heap */
        subq    $1,%rsi    /* %rsi must be odd . . . now even */
        shlq    $5,%rsi    /* top slot with children */
        movq    24(%rdi,%r10),%r15 /* %r15 = jobs pointer */
        movq    $0,(%r8)   /* set to null outside critical region */

/*          Obtain the spinlock on entry 1  */
bups:
        movb    $1,%al          /* locked value */
    xchg    %al,(%rdi,%r10) /* try to get lock on top-of-heap */
    testb   %al,%al         /* check to see if it is zero (got lock)*/
    jz      bup0            /* got the lock */
        movb    $17,%al         /* pause count if fails */
bups1:
        pause                   /* pause nicely */
        subb    $1,%al
        jnz     bups1
        jmp     bups            /* then try again */

bup0:
/*          Check if empty and return (no jobs) if so */
    movb    1(%rdi,%r10),%al /* get batch type */
    testb   %al,%al         /* is it 0 =>empty */
    jnz     bup1            /* non-empty so go to bup1p */
    movb    %al,0(%rdi,%r10) /* unlock top ASAP (%al=0) */
        xorq    %r9,%r9     /* set number of jobs as zero */
        jmp     bupexit     /* all done! */

bup1:                       /* batch type at least 1 */
    cmpb    $2,%al          /* examine batch-type */
    jae     bup2p           /* 2 or above take a jump */
    movq    $1,%r9          /* one job */
    movq    %r15,(%rcx)     /* and this is it */
    jmp     bupheap         /* deal with top empty */

bup2p:
    movq    %r15,(%r8)      /* put in "freeths" (flags unchanged) */
    jne     bup3            /* if breakable (3) go bup3 */
                            /* unbreakable batch */
    movl    16(%rdi,%r10),%r9d  /* number of jobs to move */
    movq    %r9,%rbx        /* counter of jobs to decrement */

bup23a:
    movq    (%r15),%rax
    movq    %rax,(%rcx)
    addq    $8,%r15
    addq    $8,%rcx
    subq    $1,%rbx
    jnz     bup23a

    jmp bupheap            /* sort out heap - top empty */

bup3:
    movl    16(%rdi,%r10),%ebx   /* total jobs in batch  */
    movl    20(%rdi,%r10),%eax   /* executed so far */
    movl     4(%rdi,%r10),%r9d   /* max jobs to extract */
    subq    %rax,%rbx             /* jobs left in batch  */
    cmpq    %r9,%rbx             /* compare max and left */
    jb      bup3a                /* partial extraction */
    movq    %r15,(%r8)           /* save freethis */
    movq    %rbx,%r9             /* #jobs is number left */
    leaq    (%r15,%r9,8),%r15    /* point to start of batch */
    movq    %r9,%rbx             /* counter to decrement (returned) */
    jmp     bup23a               /* do the move and adjust heap */
bup3a:
    leaq    (%r15,%r9,8),%r15    /* point to start of batch */
    movq    %r9,%rbx             /* counter to decrement (returned) */
bup3b:
    movq    (%r15),%rax          /* move in partial batch */
    movq    %rax,(%rcx)
    addq    $8,%r15
    addq    $8,%rcx
    subq    $1,%rbx
    jnz     bup3b
    xorb    %al,%al
    movb    %al,0(%rdi,%r10) /* unlock top ASAP (%al=0) */
        jmp     bupexit     /* all done! */

bupheap:
    cmpq    %r10,%rsi       /* has this slot got children */
    jae     buph1          /* yes so continue moving */
buph0:
    xorb    %al,%al
    movb    %al,0(%rdi,%r10) /* unlock top ASAP (%al=0) */
        movq    %r10,%rcx
        shrq    $6,%r10      /* divide T by 64 */
        andb    $63,%cl
        movq    $1,%rax
        shlq    %cl,%rax
      lock orq  %rax,(%rdx,%r15,8)  /* set slot as free */
        jmp     bupexit

buph1:
    movb    $1,%al          /* locked value */
    xchg    %al,(%rdi,%r10,2) /* try to get lock on 2T */
    testb   %al,%al         /* check to see if it is zero (got lock)*/
    jz      buph2          /* got the lock */
    pause
    jmp     buph1
buph2:
    movb    $1,%al          /* locked value */
    xchg    %al,64(%rdi,%r10,2) /* try to get lock on 2T+1 */
    testb   %al,%al         /* check to see if it is zero (got lock)*/
    jz      buph3          /* got the lock */
    pause
    jmp     buph2

buph3:
    movq    %r10,%rbp          /* make %rbp index of 2T */
    shlq    $1,%rbp
    movb    1(%rdi,%rbp),%al   /* get batch-type of 2T  */
    testb   %al,%al            /* is 2T empty? */
    jz      buph4              /* if so, not 2T */
    movb    65(%rdi,%rbp),%al  /* get batch-type of 2T+1  */
    testb   %al,%al            /* is 2T+1 empty? */
    jz      bupmv              /* if so, must be T */
//  neither is empty . . . . 
    movq    8(%rbp),%rax       /* urgency of 2T */
    cmpq    %rax,72(%rbp)      /* compare urgency of 2T+1 */
    jb      bupmv              /* 2T wins */
    addq    $64,%rbp           /* 2T+1 wins */
    jmp     bupmv

buph4:                         /*  2T is empty */
    movb    65(%rdi,%rbp),%al  /* get batch-type of 2T+1  */
    testb   %al,%al            /* is 2T+1 empty? */
    jz      bupx               /* if so, not this one either*/
    addq    $64,%rbp           /* 2T+1 is the winner */
    jmp     bupmv              /* do move and round again */

bupmv:                         /* going round again all locked */
    movq    %rbp,%rbx
    xorq    $64,%rbx
    xorb    %al,%al
    movb    %al,(%rbx)         /* unlock the other one */
    movb    $8,%bl
    movq    %r10,%r11
    movq    %rbp,%r15
bupmv1:
    movq    (%rdi,%r15),%rax
    movq    %rax,(%rdi,%r11)   /* both locked so OK */
    addq    $8,%r11
    addq    $8,%r15
    subb    $1,%bl
    jnz     bupmv1
    xorb    %al,%al
    movb    %al,(%rdi,%rbp)    /* unlock T */
    movq    %rbp,%r10          /* set T = X */
    jmp     bupheap

bupx:                          
    xorb    %al,%al
    movb    %al,0(%rdi,%rbp)  /* unlock 2T */
    movb    %al,64(%rdi,%rbp) /* unlock 2T+1 */   
    jmp     buph0

bupexit:
        movq    %r9,%rax
        popq    %r15
        popq    %r14
        popq    %rbx
        popq    %rbp
        ret

/* tfm3fes ( bitstring heapsize ) */
/*              %rdi     %rsi     */
/* %rax building bit number, %rcx word, %rdx bit number within word */
/* spare scratch registers r8 r9 r10 r11 */ 
	.text
	.globl	tfm3fes
tfm3fes:                     /* find empty slot routine */
        shrq    $6,%rsi      /* one less than #words in bitstring */
        addq    $1,%rsi      /* number of words in bitstring */
        xorq    %rax,%rax    /* start at word zero */
fes1:
        movq    (%rdi,%rax,8),%rcx  /* get word %rax */
        testq   %rcx,%rcx    /* "free" text - combines with jump */
        jnz     fes2         /* got a word with a free slot */
        addq    $1,%rax      /* try next word of bitstring */
        subq    $1,%rsi      /* are there any left? */
        jnz     fes1         /* if so, look at them */
        xorq    %rax,%rax    /* return zero - no empty slot */
                             /* note - slot zero is NEVER free! */
        ret                  /* all done - no free slot */
fes2:
        bsfq    %rcx,%rdx    /* find bit number of first set bit */
        shlq    $6,%rax      /* multiply word number by 64 */
        addq    %rdx,%rax    /* and add bit number within word */
        ret                  /* found a free slot */

/* end of tfm3.s  */
