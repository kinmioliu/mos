#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/*
 * send a 8/16/32-bit value on a I/O location
 * */
static inline void outb(uint16_t port, uint8_t val)
{
    /*Nd N:uint8 constriant, if port is uint8, then use outb %al, $imm8,
     * if port is u16, then outb %al, %dx*/
    __asm__ volatile (
            "outb %0, %1"
            :
            :"a"(val), "Nd"(port)
            );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile (
            "inb %1, %0"
            :"=a"(ret)
            :"Nd"(port)
            );
    return ret;
}

static inline void io_wait(void)
{
    __asm__ volatile (
            "outb %%al, $0x80"
            :
            :"a"(0)
            );
    /*
    __asm__ volatile (
            "jmp 1f\n\t"
            "1:jmp 2f\n\t"
            "2:");
            );
    */
}
