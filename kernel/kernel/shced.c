#include <stdio.h>
#include <list.h>

typedef int32_t (*task_exec)(void *argv);

struct ktask {
    uint32_t esp;
    uint32_t ebp;
    // mm space
    struct list_head link;
    char name[32];
    uint8_t state;
    task_exec exec;
    uint8_t stack[0];
};

int32_t test_task(void *argv)
{
    printk("this is a test_task.\n");
}

 struct task*__attribute__((optimize("O0")))  create_task()
{
    struct ktask *task = (struct ktask *)kmalloc_page();
    struct raw_page *new_stack = kmalloc_page();
    task->exec = test_task;
    uint32_t cur_ebp, cur_esp, cur_esi, cur_edi;
    __asm__ volatile (
        "movl %%ebp, %0;"
        "movl %%esp, %1;"
        "movl %%esi, %2;"
        "movl %%edi, %3;"
            :"=a"(cur_ebp),"=b"(cur_esp),"=c"(cur_esi),"=d"(cur_edi)
            );
    printk("cur_ebp:%x, cur_esp:%x, cur_esi:%x, cur_edi:%x\n", cur_ebp, cur_esp, cur_esi, cur_edi);
    printk("test_task:%x, new_stack:%x\n", test_task, new_stack);
    switch_task(test_task, new_stack);
    return task;

    // save cur pbc
    // switch to another pbc
}

void init_sched()
{
    printk("init sched\n");
    create_task();
}

