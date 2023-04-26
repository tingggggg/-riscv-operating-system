.section .data
mutex:
    .word 0

.section .text
.globl task_yield
.globl acquire_mutex
acquire_mutex:
    li t0, 1
    la t1, mutex
1:
    # set mutex to one
    # set t2 register by origin mutex value
    amoswap.w.aq t2, t0, (t1) 
    # origin mutex value : 1 means already be acquired
    # origin mutex valie : 0 means lock is available
    bnez t2, 1b
    ret

.globl release_mutex
release_mutex:
    li t0, 0
    la t1, mutex
    amoswap.w.aq t2, t0, (t1) # set mutex to zero (release lock)
    # jal task_yield
    la t3, task_yield
    jalr ra, t3
    ret
