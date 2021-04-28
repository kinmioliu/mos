#include <stdint.h>
#include <stdio.h>
extern void print_boot_page_table();
void __attribute__((optimize("O0"))) test_big_stack_frame()
{
    static int recursion_times = 0;
    int big_mem[1024] = {0};
    int i;
    for (i = 0; i < 1024; i++) {
        big_mem[i] = i;
    }
    recursion_times++;
    printk("recursion_times:%d, pi:%x\n", recursion_times, &i);
    if (recursion_times >= 3) {
        print_boot_page_table();
        return;
    }
    test_big_stack_frame();
}
