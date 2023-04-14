#include "platform.h"

    # size of each hart's stack is 1024 bytes
	.equ	STACK_SIZE, 1024

    .global _start

    .text
_start:
    csrr t0, mhartid                # read hart id
    mv tp, t0                       # keep CPU's hartid in its tp for later usage
    bnez t0, park                   # if we're not on the hart 0
                                    # we park the hart

    slli t0, t0, 10                 # shift left the hart id by 1024
    la sp, stacks + STACK_SIZE

    add sp, sp, t0                  # move the current hart stack pointer

    j start_kernel                  # hart 0 jump to c

park:
    wfi                             # Wait for interrupt instruction (Low power)
    j park

stacks:
    .skip STACK_SIZE * MAXNUM_CPU   # allocate space for all the harts stacks

    .end                            # end of file