# Operating System Design and Implementation

## Memory Management
WIP

***

## Context Switch

### Implementation of switch
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

### Created two processes
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


### result

![Switch Result](./04-multitask/image/result.png)

***

## Exception & Interrupt handling

* Three types of interrupt
    * Software interrupt
    * Timer interrupt
    * Externel interrupt

![Interrupt Arch Block](./06-interrupts/images/interrupt_arch_block.png)

<img src="./06-interrupts/images/mcause_regs.png" alt="Mcause regs" width="400">

* Machine Interrupt Enable Register (mie)

<img src="./06-interrupts/images/mie.png" alt="MIE regs" width="400">

* PLIC

<img src="./06-interrupts/images/plic.png" alt="plic" width="400">
<img src="./06-interrupts/images/plic_map.png" alt="plic map" width="600">

```c
/* 
 * Set priority for UART0.
 *
 * Each PLIC interrupt source can be assigned a priority by writing 
 * to its 32-bit memory-mapped priority register.
 */
*(uint32_t *) PLIC_PRIORITY(UART0_IRQ) = 1;
```
<img src="./06-interrupts/images/plic_priority.png" alt="plic priority" width="600">

```c
#define PLIC_MCLAIM(hart) (PLIC_BASE + 0x200004 + (hart) * 0x1000)
#define PLIC_MCOMPLETE(hart) (PLIC_BASE + 0x200004 + (hart) * 0x1000)

/* 
 * DESCRIPTION:
 *	Query the PLIC what interrupt we should serve.
 */
int plic_claim(void)
{
    int hart = r_tp();
    int irq = *(uint32_t *) PLIC_MCLAIM(hart);
    return irq;
}

/* 
 * DESCRIPTION:
  *	Writing the interrupt ID it received from the claim (irq) to the 
 *	complete register would signal the PLIC we've served this IRQ. 
 */
void plic_complete(int irq)
{
    int hart = r_tp();
    *(uint32_t *) PLIC_MCOMPLETE(hart) = irq;
}

void external_interrupt_handler()
{
    int irq = plic_claim();

    if (irq == UART0_IRQ) {
        uart_isr();
    } else if (irq) {
        printf("Unexpected interrupt irq = %d\n", irq);
    }

    if (irq) {
        plic_complete(irq);
    }
}

reg_t trap_handler(reg_t epc, reg_t cause)
{
    reg_t return_pc = epc;
    reg_t cause_code = cause & 0xfff;

    if (cause & 0x80000000) {
        switch (cause_code)
        {
        ...
        case 11:
            uart_puts("external interuption!\n");
            external_interrupt_handler();
            break;
        default:
            break;
        }
    } else {
        /* Synchronous trap - exception */
        printf("Sync exceptions!, code = %d\n", cause_code);
        panic("PANIC");
        // return_pc += 4;
    }
    
    return return_pc;
}
```
<img src="./06-interrupts/images/plic_claim_complete.png" alt="plic claim complete" width="600">

### result

![Interruput result](./06-interrupts/images/result.png)
