#include <stdio.h>
#include <list.h>

typedef int32_t (*task_exec)(void *argv);

struct ktask {
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eip;
    // mm space
    //struct list_head link;
    //char name[32];
    uint8_t state;
    task_exec exec;
    uint8_t stack[0];
};
#define MAX_TASK 10

struct ktask* ktask_queue[10] = {0};
uint32_t task_num = 0;
struct ktask* cur_ktask;
uint32_t g_eip;

extern uint32_t g_pic_count;
extern long task1_eip;
extern long task1_ebp;
extern long task1_esp;
extern long task2_ebp;

int32_t test_task(void *argv)
{
    cur_ktask = (struct ktask*)0x1000;
    printk("this is a test_task %d,%x,%x,%x,%x.\n", g_pic_count, task1_eip, task1_esp, task1_ebp, task2_ebp);
// /*   
    while (1) {
        if (g_pic_count % 10 == 0) {            
            printk("1test_task is running %d.\n", g_pic_count);
            printk("2test_task is running %d.\n", g_eip);
        }
    };
    printk("test_task end.\n");
//   */ 
}

int32_t test_task2(void *argv)
{
    cur_ktask = (struct ktask*)0x2000;
    printk("this is a test_task2 %d,%x,%x,%x,%x.\n", g_pic_count, task1_eip, task1_esp, task1_ebp, task2_ebp);
// /*  
    while (1) {
        if (g_pic_count % 10 == 0) {            
            printk("3test_task is running %d.\n", g_pic_count);
            printk("4test_task is running %d.\n", g_pic_count);
        }
    };
    printk("test_task2 end.\n");
// */  
}


struct ktask* get_next_task() 
{
    static uint32_t i = 0;
    i++;
    if (i % 2 == 0) {
        return ktask_queue[0];
    } else {
        return ktask_queue[1];
    }
}

/*
void sched_task(struct *ktask)
{
    run_task(
    switch_task(ktask);
    1. push register to cur_task stack
    2. set cur task_stack to idle state
    3. switch to next task
    3.0 set esp
    3.1 pop(restore) register
    3.2 run 

}
*/

void __attribute__((optimize("O0"))) pick_next_task()
{
    struct ktask* next_task = get_next_task();
    if (next_task != 0) {
        /* this may be the kernel stack*/
        uint32_t cur_ebp, cur_esp, cur_esi, cur_edi;
         //cur_ktask = next_task;
         //printk("cur_task:%x, eip:%x, esp:%x\n", cur_ktask, cur_ktask->eip, cur_ktask->esp);
         if (next_task->eip == test_task) {
         __asm__ volatile (
            "movl %%ebp, %0;"
            "movl %%esp, %1;"
            "movl %%esi, %2;"
            "movl %%edi, %3;"
//                :"=a"(cur_ktask->ebp),"=b"(cur_ktask->esp),"=c"(cur_ktask->esi),"=d"(cur_ktask->edi)
                  :"=a"(cur_ebp),"=b"(cur_esp),"=c"(cur_esi),"=d"(cur_edi)
                );
         printk(" pick next task cur_ebp:%x, cur_esp:%x, cur_esi:%x, cur_edi:%x\n", cur_ebp, cur_esp, cur_esi, cur_edi);
            printk("task2 to task1\n");
             run_task2(next_task->eip, next_task->esp);
             printk("task2 finish switch\n");
         } else {
          __asm__ volatile (
            "movl %%ebp, %0;"
            "movl %%esp, %1;"
            "movl %%esi, %2;"
            "movl %%edi, %3;"
//                :"=a"(cur_ktask->ebp),"=b"(cur_ktask->esp),"=c"(cur_ktask->esi),"=d"(cur_ktask->edi)
                  :"=a"(cur_ebp),"=b"(cur_esp),"=c"(cur_esi),"=d"(cur_edi)
                );
         printk(" pick next task cur_ebp:%x, cur_esp:%x, cur_esi:%x, cur_edi:%x\n", cur_ebp, cur_esp, cur_esi, cur_edi);
           printk("task1 to task2\n");
             run_task3(next_task->eip, next_task->esp);
             cur_ktask = (struct ktask*)0x1000;
             printk("task1 finish switch\n");
              __asm__ volatile (
                "movl %%ebp, %0;"
                "movl %%esp, %1;"
                "movl %%esi, %2;"
                "movl %%edi, %3;"
                      :"=a"(cur_ebp),"=b"(cur_esp),"=c"(cur_esi),"=d"(cur_edi)
                    );
               printk(" pick next task cur_ebp:%x, cur_esp:%x, cur_esi:%x, cur_edi:%x\n", cur_ebp, cur_esp, cur_esi, cur_edi);
               return;
         }
    }
}

 struct task*__attribute__((optimize("O0")))  create_task()
{
    struct ktask *task = (struct ktask *)kmalloc_page();
    if (task_num == 1) {
        task->exec = test_task2;
    } else {
        task->exec = test_task;
    }
    uint32_t cur_ebp, cur_esp, cur_esi, cur_edi;
    __asm__ volatile (
        "movl %%ebp, %0;"
        "movl %%esp, %1;"
        "movl %%esi, %2;"
        "movl %%edi, %3;"
            :"=a"(cur_ebp),"=b"(cur_esp),"=c"(cur_esi),"=d"(cur_edi)
            );
    printk("cur_ebp:%x, cur_esp:%x, cur_esi:%x, cur_edi:%x\n", cur_ebp, cur_esp, cur_esi, cur_edi);
    printk("param:%x,%x\n", task->exec, (uint32_t)(task) + 0x900);
    init_task(task->exec, (uint32_t)(task) + 0x900);
    printk("init task:%x\n", task_num);
    task->esp = (uint32_t)(task) + 0x900;
    task->eip = task->exec;
    if (task_num == 0) {
        cur_ktask = task;
    }
    ktask_queue[task_num++] = task;
    cur_ktask = task;
    return task;

    // save cur pbc
    // switch to another pbc
}

void schedule()
{
    while (g_pic_count % 75 == 0)
    {
        pick_next_task();
    }
}

void init_sched()
{
    printk("init sched\n");
    create_task();
    create_task();
    //schedule();
    uint32_t eip = read_eip();
    if (cur_ktask == 0x1000) {
        g_eip = eip;
    } else {
        g_eip = 5;
    }
    printk("ret:%x\n", eip);
    int *test;
    if ((uint32_t)(0xC0000000) < (uint32_t)(&test)) {
        printk("less:%x\n",(uint32_t)(&test));
    }
    
}

