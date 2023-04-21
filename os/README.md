# Operating System Design and Implementation

## Memory Management
WIP

***

## Context Switch

#### Implementation of switch
```asm
.macro reg_save base
	sw ra, 0(\base)
	sw sp, 4(\base)
	sw gp, 8(\base)
	sw tp, 12(\base)
	sw t0, 16(\base)
	sw t1, 20(\base)
    ...
	sw t5, 116(\base)
	# we don't save t6 here, due to we have used
	# it as base, we have to save t6 in an extra step
	# outside of reg_save
.endm

# restore all General-Purpose(GP) registers from the context
# struct context *base = &ctx_task;
# ra = base->ra;
# ......
.macro reg_restore base
	lw ra, 0(\base)
	lw sp, 4(\base)
	lw gp, 8(\base)
	lw tp, 12(\base)
	lw t0, 16(\base)
	lw t1, 20(\base)
    ...
	lw t5, 116(\base)
	lw t6, 120(\base)
.endm

# void switch_to(struct context *next);
# a0: pointer to the context of the next task
.globl switch_to
.align 4
switch_to:
    csrrw t6, mscratch, t6      # swap t6 and mscratch
    beqz t6, 1f

    reg_save t6                 # save context of prev task

	# Save the actual t6 register, which we swapped into
	# mscratch
    mv t5, t6
    csrr t6, mscratch
    sw t6, 120(t5)
1:
    # switch mscratch to point to the context of the next task
    csrw mscratch, a0

    # Restore all GP registers
    # Use t6 to point to the context of the task0
    mv t6, a0
    reg_restore t6

    ret
```

#### Created two processes
```c
void user_task0(void)
{
    uart_puts("Task 0: Created!\n");
    char s[] = "12345";
    while (1) {
        for (int i = 0; i < 5; i++) {
            uart_putc(s[i]);
            task_delay(DELAY);
            task_yield();
        }
    }
}

void user_task1(void)
{
    uart_puts("Task 1: Created!\n");
    char s[] = "abcde";
    while (1) {
        for (int i = 0; i < 5; i++) {
            uart_putc(s[i]);
            task_delay(DELAY);
            task_yield();
        }
    }
}
```


#### resuls

![Switch Result](https://github.com/tingggggg/riscv-operating-system/blob/main/os/04-multitask/image/result.png)

***

## Interrupt handling
WIP

