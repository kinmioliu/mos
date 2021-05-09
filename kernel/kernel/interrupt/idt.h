#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

/* inline asm interrupt instruction */
static inline void hlt()
{
    __asm__ volatile("hlt");
}

static inline void cli()
{
    __asm__ volatile("cli");
}

static inline void sti()
{
    __asm__ volatile("sti");
}

/* om function */
void print_idt();

/* module function */
void init_interrupt();

#endif
