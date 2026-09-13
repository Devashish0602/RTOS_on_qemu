    .section .text.init
    .global _start
_start:
    csrw    mie, zero                # all interrupts off for now
    la      sp, _stack_top
    la      t0, trap_entry
    csrw    mtvec, t0

    la      t0, __bss_start          # zero .bss
    la      t1, __bss_end
1:  bgeu    t0, t1, 2f
    sw      zero, 0(t0)
    addi    t0, t0, 4
    j       1b

2:  call    main

3:  wfi
    j       3b

    .section .text
    .align 4
.global trap_entry
trap_entry:

addi    sp, sp, -128            # allocate frame first: offsets stay positive
 
    sw      x1,    0(sp)            # x0 hardwired, x2 (sp) lives in the TCB
    sw      x3,    4(sp)
    sw      x4,    8(sp)
    sw      x5,   12(sp)
    sw      x6,   16(sp)
    sw      x7,   20(sp)
    sw      x8,   24(sp)
    sw      x9,   28(sp)
    sw      x10,  32(sp)
    sw      x11,  36(sp)
    sw      x12,  40(sp)
    sw      x13,  44(sp)
    sw      x14,  48(sp)
    sw      x15,  52(sp)
    sw      x16,  56(sp)
    sw      x17,  60(sp)
    sw      x18,  64(sp)
    sw      x19,  68(sp)
    sw      x20,  72(sp)
    sw      x21,  76(sp)
    sw      x22,  80(sp)
    sw      x23,  84(sp)
    sw      x24,  88(sp)
    sw      x25,  92(sp)
    sw      x26,  96(sp)
    sw      x27, 100(sp)
    sw      x28, 104(sp)
    sw      x29, 108(sp)
    sw      x30, 112(sp)
    sw      x31, 116(sp)

    csrr    t0, mepc                
    sw      t0,  120(sp)
    csrr    t0, mstatus
    sw      t0,  124(sp)
 

    csrr t0, mcause
    bltz t0, interrupt_handler

    call trap_handler
    j   restore 

interrupt_handler:
    mv a0, sp
    call context_switch
    mv sp, a0
    j   restore

restore:

// restoring values from stack
    lw      t0,  124(sp)
    csrw    mstatus, t0             
    lw      t0,  120(sp)
    csrw    mepc, t0
 
    lw      x1,    0(sp)
    lw      x3,    4(sp)
    lw      x4,    8(sp)
    lw      x5,   12(sp)
    lw      x6,   16(sp)
    lw      x7,   20(sp)
    lw      x8,   24(sp)
    lw      x9,   28(sp)
    lw      x10,  32(sp)
    lw      x11,  36(sp)
    lw      x12,  40(sp)
    lw      x13,  44(sp)
    lw      x14,  48(sp)
    lw      x15,  52(sp)
    lw      x16,  56(sp)
    lw      x17,  60(sp)
    lw      x18,  64(sp)
    lw      x19,  68(sp)
    lw      x20,  72(sp)
    lw      x21,  76(sp)
    lw      x22,  80(sp)
    lw      x23,  84(sp)
    lw      x24,  88(sp)
    lw      x25,  92(sp)
    lw      x26,  96(sp)
    lw      x27, 100(sp)
    lw      x28, 104(sp)
    lw      x29, 108(sp)
    lw      x30, 112(sp)
    lw      x31, 116(sp)
 
    addi    sp, sp, 128
    mret