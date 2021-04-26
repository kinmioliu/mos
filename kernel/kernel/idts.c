/*
 * idt service
 * 1. register idt
 * 2. handler
 * */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <io.h>

void dividezero_handler()
{
    printk("divide_zero\n");
    uint8_t v = inb(0x60);
    PIC_sendEOI(1);
}

struct idtDesc {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_2;
};


struct idtDesc idt[0xff] = {0};



extern void set_idt();
void register_idt(int id, uint32_t isr)
{
    // create a idt entry
    printk("id:%x, isr:%x\n", id, isr);
    idt[0].offset_1 = (uint16_t)(((uint32_t)dividezero_handler) & 0xffff);
    idt[0].selector = 0x8; //km code
    idt[0].zero = 0;
    idt[0].type_attr = 0x8e;// 0b10001110
    idt[0].offset_2 = (uint16_t)(((uint32_t)dividezero_handler) >> 16);
    // register
    printk("%x,%x,%x,%x, %x\n", idt[0].offset_1, idt[0].selector, idt[0].zero, idt[0].type_attr, idt[0].offset_2);
    set_idt(idt);
}

void test_idts()
{
    printk("test_idts\n");
    register_idt(0, 0);
    int a = 5;
    int b = 0;
    //int c = a / b;
    /*
    __asm__ volatile(
    "int $0x2;"
    );
    */
    /*
    printk("a page fault interrupt\n");
    int *p = 0x8b;
    *p = 5;
    */
    //printk("c is :%x\n", c);
}


