/*
 * idt service
 * 1. register idt
 * 2. handler
 * */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <io.h>
#include <idts.h>
#include <pic.h>
#include <idt.h>
#include <isr_stub.def>

extern void init_divide_zero();
extern void init_keyboard();
extern void init_pagefault();
extern void init_pit();
//#define MAX_INTERRUPT   0x100
#define PIC1_REMAP_OFFSET 0x20
#define PIC2_REMAP_OFFSET 0x28


typedef struct IdtDescTag {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_2;
} IdtDesc;

uint8_t g_idtr[6] = {0};
IdtDesc g_idt[MAX_INTERRUPT] = {0};

void register_isr(uint16_t iid, uint32_t handler)
{
    if (iid > MAX_INTERRUPT) {
        return;
    }
    g_idt[iid].offset_1 = (uint16_t)(handler & 0xffff);
    g_idt[iid].selector = 0x8; //km code
    g_idt[iid].zero = 0;
    g_idt[iid].type_attr = 0x8e;// 0b10001110
    g_idt[iid].offset_2 = (uint16_t)(handler >> 16);
    printk("registr iid:%x handler:%x success.\n", iid, handler);
}

void create_idt()
{
    int i;
    for (i = 0; i < MAX_INTERRUPT; i++) {
        g_idt[i].offset_1 = (uint16_t)(((uint32_t)(global_isr_stub_func[i])) & 0xffff);
        g_idt[i].selector = 0x8; //km code
        g_idt[i].zero = 0;
        g_idt[i].type_attr = 0x8e;// 0b10001110
        g_idt[i].offset_2 = (uint16_t)(((uint32_t)(global_isr_stub_func[i])) >> 16);
    }
    // register special idt 
    init_divide_zero();
    init_keyboard();
    init_pagefault();
    init_pit();
}

void set_idtr()
{
    uint16_t size = MAX_INTERRUPT * sizeof(IdtDesc) - 1;
    *(uint16_t *)g_idtr = size;
    *(uint32_t *)(g_idtr + 2) = (uint32_t)g_idt;
    //printk("%x %x %x %x %x %x\n", g_idtr[0], g_idtr[1],g_idtr[2], g_idtr[3],g_idtr[4],g_idtr[5]);
}

void init_interrupt()
{
    PIC_remap(PIC1_REMAP_OFFSET, PIC2_REMAP_OFFSET);
    create_idt();
    set_idtr();
    __asm__ volatile(
            "movl %0, %%eax\t\n"
            "lidt (%%eax)\t\n"
            :
            :"r"(g_idtr));
}

