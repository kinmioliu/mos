#include <idts.h>
#include <io.h>
#include <idt.h>
#include <stdint.h>
#include <stddef.h>


uint16_t read_pit_count(void) {
	uint16_t count = 0;
 
	// Disable interrupts
	cli();
 
	// al = channel in bits 6 and 7, remaining bits clear
	outb(0x43,0b0000000);
 
	count = inb(0x40);		// Low byte
	count |= inb(0x40)<<8;		// High byte
 
	return count;
}

void set_pit_count(uint16_t count) {
	// Disable interrupts
	cli();
 
	// Set low byte
	outb(0x40,count&0xFF);		// Low byte
	outb(0x40,(count&0xFF00)>>8);	// High byte
    sti();
	return;
}

extern void pick_next_task();
volatile uint32_t g_pic_count = 0;
#if 0
void PIT_handler()
{
    __asm__ volatile (
//        "pusha\t\n"
        "push %ds\t\n"
        "push %es\t\n"
        "push %fs\t\n"
        "push %gs\t\n"
        "cli\t\n"                  
        );
    PIC_sendEOI(0);
    g_pic_count++;
    //printk("inter\n");
    if (g_pic_count % 20 == 0) {
        cli();
        pick_next_task();
        sti();
    }
    __asm__ volatile (
        "sti\t\n"
        "pop %gs\t\n"
        "pop %fs\t\n"
        "pop %es\t\n"
        "pop %ds\t\n"
//        "popa\t\n"
        "iret\t\n"
        );        
}
#endif

__attribute__((interrupt)) void PIT_handler(interrupt_frame_t frame)
{
/*    
    __asm__ volatile (
//        "pusha\t\n"
        "push %ds\t\n"
        "push %es\t\n"
        "push %fs\t\n"
        "push %gs\t\n"
        "cli\t\n"          
        );
*/        
    PIC_sendEOI(0);
    g_pic_count++;
//    printk("inter\n");
    if (g_pic_count % 75 == 0) {
        cli();
        pick_next_task();
        sti();
    }
/*    
    __asm__ volatile (
        "sti\t\n"
        "pop %gs\t\n"
        "pop %fs\t\n"
        "pop %es\t\n"
        "pop %ds\t\n"
//        "popa\t\n"
//        "iret\t\n"
        );
*/        
}

void init_pit()
{
    register_isr(0x20, (uint32_t)PIT_handler);
}
