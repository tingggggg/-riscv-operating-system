#include "os.h"

extern void trap_vector(void);

void trap_init()
{
    /*
	 * set the trap-vector base-address for machine-mode
	 */
    w_mtvec((reg_t) trap_vector);
}

reg_t trap_handler(reg_t epc, reg_t cause)
{
    reg_t return_pc = epc;
    reg_t cause_code = cause & 0xfff;

    if (cause & 0x80000000) {
        switch (cause_code)
        {
        case 3:
            uart_puts("software interruption!\n");
            break;
        case 7:
            uart_puts("timer interruption!\n");
            break;
        case 11:
            uart_puts("external interuption!\n");
        default:
            break;
        }
    } else {
        /* Synchronous trap - exception */
        printf("Sync exceptions!, code = %d\n", cause_code);
        // panic("PANIC");
        return_pc += 4;
    }
    
    return return_pc;
}

void trap_test()
{
    /*
	 * Synchronous exception code = 7
	 * Store/AMO access fault
	 */
    *(int *) 0x00000000 = 100;

    /*
	 * Synchronous exception code = 5
	 * Load access fault
	 */
	//int a = *(int *)0x00000000;

    uart_puts("Return back from trap!\n");
}
