#include <stddef.h>
#include <stdint.h>
#include <io.h>

#define PIC1    0x20
#define PIC2    0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA   (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA   (PIC2 + 1)

#define PIC_EOI     0x20

/*ICW:Initialisation Command Word
 *OCW:Operational Command Word
 * */
#define ICW1_ICW4       0x01
#define ICW1_SINGLE     0x02
#define ICW1_INTERVAL4  0x04
#define ICW1_LEVEL      0x08
#define ICW1_INIT       0x10

#define ICW4_8086       0x01
#define ICW4_AUTO       0x02
#define ICW4_BUF_SLAVE  0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM       0x10

/*
 * ISR and IRR
 * */
#define PIC_READ_IRR    0x0a
#define PIC_READ_ISR    0x0b


void PIC_sendEOI(unsigned char irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void PIC_remap(int offset1, int offset2)
{
    uint8_t a1, a2;
    printk("PIC_remap:offset1:%x, offset2:%x\n", offset1, offset2);
    asm("cli");
    a1 = inb(PIC1_DATA);    // save masks
    a2 = inb(PIC2_DATA);    

    // send init to pic1 and pic2
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // set pic1 interrupt port to offset1
    outb(PIC1_DATA, offset1);       // ICW2: Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, offset2);       // ICW2: Slave PIC vector offset
    io_wait();
    outb(PIC1_DATA, 4);         // ICW3: tell Master PIC that there is a slave PIC at IRQ2(0000 0100)
    io_wait();
    outb(PIC2_DATA, 2);         // ICW3: tell Slave its cascade identity(0000 0010)
    io_wait();
    
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1);    // restore saved masks
    outb(PIC2_DATA, a2);
    // set keybood 
    // IRQ_set_mask(1);
    //IRQ_clear_mask(1);
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
    IRQ_clear_mask(1);
    io_wait();
    asm("sti");
}

void IRQ_set_mask(uint8_t IRQline) 
{
    uint16_t port;
    uint8_t value;

    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);
}

void IRQ_clear_mask(uint8_t IRQline) 
{
    uint16_t port;
    uint8_t value;
    
    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);
}

static uint16_t __pic_get_irq_reg(int ocw3)
{
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}

void keyboard_handler(int a, int b)
{
    printk("a:%d, b:%d\n",a ,b);
    //outb(PIC1_COMMAND, 0x61);
    // get the keyboard value
    uint8_t val = inb(0x60);
    printk("keyboard:%x\n", val);
    PIC_sendEOI(1);
}
