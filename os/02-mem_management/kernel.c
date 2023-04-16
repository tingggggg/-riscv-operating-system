#include "os.h"

extern void uart_init(void);
extern void uart_puts(char *s);

void start_kernel(void)
{
    uart_init();
    uart_puts("Hello, RVOS!\n");

    page_init();
    
    while (1) {}; // loop here
}