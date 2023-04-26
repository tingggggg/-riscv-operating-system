#include "os.h"

#define DELAY 4000
// #define USE_SPIN_LOCK
#define USE_MUTEX

extern void trap_test();

extern int spin_lock();
extern int spin_unlock();

extern void acquire_mutex(void);
extern void release_mutex(void);

void user_task0(void)
{
    uart_puts("Task 0: Created!\n");
    char s[] = "12345";

    // task_yield();
    // uart_puts("Task 0: I'm back!\n");
    while (1) {
#ifdef USE_MUTEX
        acquire_mutex();
#endif
        for (int i = 0; i < 5; i++) {
            uart_putc(s[i]);
            task_delay(DELAY);
        }
#ifdef USE_MUTEX
        release_mutex();
#endif
    }
}

void user_task1(void)
{
    uart_puts("Task 1: Created!\n");
    char s[] = "abcde";
    while (1) {
#ifdef USE_SPIN_LOCK
        spin_lock();
#endif
#ifdef USE_MUTEX
        acquire_mutex();
#endif
        for (int i = 0; i < 5; i++) {
            uart_putc(s[i]);
            task_delay(DELAY);
        }
#ifdef USE_SPIN_LOCK
        spin_unlock();
#endif
#ifdef USE_MUTEX
        release_mutex();
#endif
    }
}

void os_main(void)
{
    task_create(user_task0);
    task_create(user_task1);
}
