#include <idts.h>
#include <pic.h>
#include <io.h>
#include <stdio.h>
/*
 *TODO: long time pressed:continue shift+AAAAAA

 * */
uint8_t g_scancode_US_QWERY[0xff] = {
};

static uint8_t scancode_buf[20] = {0};
static uint8_t bufInx = 0;

enum KeyboardState {
    INIT,
    PRESSED,
    RELEASED,
    MULTI_KEY,
};

static enum KeyboardState state = INIT;

void print_keyboard()
{
    if (bufInx % 2 != 0) {
        printk("error keybuf\n");
        bufInx = 0;
        return;
    }

    int i;
    for (i = 0; i < bufInx; i++) {
        printk("%x ", scancode_buf[i]);
    }
    printk("\n");
    bufInx = 0;
    state = INIT;
    return;
   
    if (bufInx == 2) {
       // putchar(g_scancode_US_QWERY[scancode_buf[0]]);
    }
    if (bufInx == 4) {

    }
}

void keyboard_print_fsm(uint8_t key)
{
    if (state != MULTI_KEY) {
        switch (key) {
            case 0x1 ... 0x58:
                state = PRESSED;
                break;
            case 0x81 ... 0xD8:
                state = RELEASED;
                break;
            case 0xE0:
                state = MULTI_KEY; // need recive next key
                break;
            default:
                printk("keyboard fsm error:0x%x\n", key);
                break;
        }
    } else {
        switch (key) {
            case 0x10 ... 0x6D:
                state = PRESSED;
                break;
            case 0x90 ... 0xED:
                state = RELEASED;
                break;
            default:
                printk("keyboard fsm multikey error:0x%x\n", key);
                break;
        }
    }
    scancode_buf[bufInx++] = key;
            
    switch (state) {        
        case PRESSED:
            break;
        case RELEASED:
            // if bufInx is empty?
            if (bufInx % 2 == 0) {
                // a real release
                print_keyboard();
            }
        case MULTI_KEY:
            break;
        default:
            printk("unknow state:%x\n", state);
    }
}

void broadcast_keyboard_evt(uint8_t key_evt)
{
    // TODO: a real broadcast mechanism
    keyboard_print_fsm(key_evt);
}

__attribute__ ((interrupt)) void keyboard_handler(interrupt_frame_t frame)
{
    (void)frame;
    uint8_t val = inb(0x60);
    broadcast_keyboard_evt(val);
    PIC_sendEOI(1);
}

void init_keyboard()
{
    register_isr(0x21, (uint32_t)keyboard_handler);
}
