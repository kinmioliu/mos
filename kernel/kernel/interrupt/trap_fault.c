#include <stdint.h>
#include <idts.h>
#include <stdio.h>

__attribute__((interrupt)) void dividezero_handler(interrupt_frame_t frame)
//void dividezero_handler()
{
    __asm__ volatile (
//        "pusha\t\n"
        "push %ds\t\n"
        "push %es\t\n"
        "push %fs\t\n"
        "push %gs\t\n"
        "cli\t\n"          
        );
    printk("divide_zero\n");
    __asm__ volatile (
        "sti\t\n"
        "pop %gs\t\n"
        "pop %fs\t\n"
        "pop %es\t\n"
        "pop %ds\t\n"
//        "popa\t\n"
//        "iret\t\n"
        );
}

void init_divide_zero()
{
    register_isr(0, (uint32_t)dividezero_handler);
}


