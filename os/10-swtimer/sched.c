#include "os.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

#define MAX_TASKS 10
#define STACK_SIZE 1024
uint8_t task_stack[MAX_TASKS][STACK_SIZE];
struct context ctx_tasks[MAX_TASKS];

/*
 * _top is used to mark the max available position of ctx_tasks
 * _current is used to point to the context of current task
 */
static int _top = 0;
static int _current = -1;

static void w_mcsratch(reg_t x)
{
    asm volatile("csrw mscratch, %0" : : "r" (x));
}


void sched_init()
{
    w_mcsratch(0);

    /* enable machine-mode software interrupts. */
	w_mie(r_mie() | MIE_MSIE);
}

void schedule()
{
    if (_top <= 0) {
        panic("Number of task should be greater than 0!\n");
    }

    _current = (_current + 1) % _top;
    struct  context *next = &(ctx_tasks[_current]);
    switch_to(next);
}

/*
 * DESCRIPTION
 * 	Create a task.
 * 	- start_routin: task routine entry
 * RETURN VALUE
 * 	0: success
 * 	-1: if error occured
 */
int task_create(void (* start_routin) (void))
{
    if (_top < MAX_TASKS) {
        ctx_tasks[_top].sp = (reg_t) &task_stack[_top][STACK_SIZE - 1];
		ctx_tasks[_top].pc = (reg_t) start_routin;
		_top++;
		return 0;
    } else {
        return -1;
    }
}

void task_yield()
{   
    /* trigger a machine-level software interrupt */
    int id = r_mhartid();
    *(uint32_t *) CLINT_MSIP(id) = 1;
}

/*
 * a very rough implementaion, just to consume the cpu
 */
void task_delay(volatile int count)
{
	count *= 50000;
	while (count--);
}
