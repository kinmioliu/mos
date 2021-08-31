#include <stdio.h>
#include <io.h>

void detect_floppy()
{
    outb(0x70, 0x10);
    uint8_t drivers = inb(0x71);
    printk("drivers:%x\n", drivers);
}

//
// The MSR byte: [read-only]
// -------------
//
//  7   6   5    4    3    2    1    0
// MRQ DIO NDMA BUSY ACTD ACTC ACTB ACTA
//
// MRQ is 1 when FIFO is ready (test before read/write)
// DIO tells if controller expects write (1) or read (0)
//
// NDMA tells if controller is in DMA mode (1 = no-DMA, 0 = DMA)
// BUSY tells if controller is executing a command (1=busy)
//
// ACTA, ACTB, ACTC, ACTD tell which drives position/calibrate (1=yes)
//
//
/*
#define FLOPPY_MSR 4
#define FLOPPY_FIFO 5
#define FLOPPY_DOR 2
#define FLOPPY_CCR 7
*/
// standard base address of the primary floppy controller
static const int floppy_base = 0x03f0;
// standard IRQ number for floppy controllers
static const int floppy_irq = 6;

// The registers of interest. There are more, but we only use these here.
enum floppy_registers {
   FLOPPY_DOR  = 2,  // digital output register
   FLOPPY_MSR  = 4,  // master status register, read only
   FLOPPY_FIFO = 5,  // data FIFO, in DMA operation for commands
   FLOPPY_CCR  = 7   // configuration control register, write only
};

// The commands of interest. There are more, but we only use these here.
enum floppy_commands {
   CMD_SPECIFY = 3,            // SPECIFY
   CMD_WRITE_DATA = 5,         // WRITE DATA
   CMD_READ_DATA = 6,          // READ DATA
   CMD_RECALIBRATE = 7,        // RECALIBRATE
   CMD_SENSE_INTERRUPT = 8,    // SENSE INTERRUPT
   CMD_SEEK = 15,              // SEEK
};

void floppy_write_cmd(int base, int cmd)
{
    int i;
    for (i = 0; i < 600; i++) {
        tmp_sleep();
        if (0x80 & inb(base + FLOPPY_MSR)) {
            return (void)outb(base + FLOPPY_FIFO, cmd);
        }
    }
    printk("error!\n");
}

uint8_t floppy_read_data(int base)
{
    int i;
    for (i = 0; i < 600; i++) {
        tmp_sleep();
        if (0x80 & inb(base + FLOPPY_MSR)) {
            return inb(base + FLOPPY_FIFO);
        }
    }
    printk("read error!\n");
    return 0;
}

void floppy_check_interrupt(int base, int *st0, int *cyl)
{
    floppy_write_cmd(base, 8);
    *st0 = floppy_read_data(base);
    *cyl = floppy_read_data(base);
}

int floppy_calibrate(int base)
{
    int i, st0, cyl = -1;
    floppy_motor(base, 1);
    for (i = 0; i < 10; i++) {
        floppy_write_cmd(base, CMD_RECALIBRATE);
        floppy_write_cmd(base, 0);
        tmp_sleep();
        //irq_wait(floppy_irq);
        floppy_check_interrupt(base, &st0, &cyl);

        if (st0 & 0xC0) {
            printk("floppy calibrate status:%x\n", st0);
            continue;
        }
        
        if (!cyl) {
            floppy_motor(base, 0);
            return 0;
        }
    }

    printk("10 times retried!\n");
    return -1;
}

int floppy_reset(int base) {

    outb(base + FLOPPY_DOR, 0x00); // disable controller
    outb(base + FLOPPY_DOR, 0x0C); // enable controller

    tmp_sleep();
    //irq_wait(floppy_irq); // sleep until interrupt occurs

    {
        int st0, cyl; // ignore these here..
        floppy_check_interrupt(base, &st0, &cyl);
        printk("st0:%x, cyl:%x\n", st0, cyl);
    }


    // set transfer speed 500kb/s
    outb(base + FLOPPY_CCR, 0x00);

    //  - 1st byte is: bits[7:4] = steprate, bits[3:0] = head unload time
    //  - 2nd byte is: bits[7:1] = head load time, bit[0] = no-DMA
    //
    //  steprate    = (8.0ms - entry*0.5ms)*(1MB/s / xfer_rate)
    //  head_unload = 8ms * entry * (1MB/s / xfer_rate), where entry 0 -> 16
    //  head_load   = 1ms * entry * (1MB/s / xfer_rate), where entry 0 -> 128
    //
    floppy_write_cmd(base, 3); //CMD_SPECIFY);
    floppy_write_cmd(base, 0xdf); /* steprate = 3ms, unload time = 240ms */
    floppy_write_cmd(base, 0x02); /* load time = 16ms, no-DMA = 0 */

    // it could fail...
    if(floppy_calibrate(base)) return -1;
    printk("reset ok!\n");
    return 0;
}

//
// The DOR byte: [write-only]
// -------------
//
//  7    6    5    4    3   2    1   0
// MOTD MOTC MOTB MOTA DMA NRST DR1 DR0
//
// DR1 and DR0 together select "current drive" = a/00, b/01, c/10, d/11
// MOTA, MOTB, MOTC, MOTD control motors for the four drives (1=on)
//
// DMA line enables (1 = enable) interrupts and DMA
// NRST is "not reset" so controller is enabled when it's 1
//
enum { floppy_motor_off = 0, floppy_motor_on, floppy_motor_wait };
static volatile int floppy_motor_ticks = 0;
static volatile int floppy_motor_state = 0;

void floppy_motor(int base, int onoff) {

    if(onoff) {
        if(!floppy_motor_state) {
            // need to turn on
            outb(base + FLOPPY_DOR, 0x1c);
            tmp_sleep();
            //timer_sleep(50); // wait 500 ms = hopefully enough for modern drives
        }
        floppy_motor_state = floppy_motor_on;
    } else {
        if(floppy_motor_state == floppy_motor_wait) {
            printk("floppy_motor: strange, fd motor-state already waiting..\n");
        }
        floppy_motor_ticks = 300; // 3 seconds, see floppy_timer() below
        floppy_motor_state = floppy_motor_wait;
    }
}

void floppy_motor_kill(int base) {
    outb(base + FLOPPY_DOR, 0x0c);
    floppy_motor_state = floppy_motor_off;
}

//
// THIS SHOULD BE STARTED IN A SEPARATE THREAD.
//
//
void floppy_timer() {
    while(1) {
        // sleep for 500ms at a time, which gives around half
        // a second jitter, but that's hardly relevant here.
        //timer_sleep(50);
        tmp_sleep();
        if(floppy_motor_state == floppy_motor_wait) {
            floppy_motor_ticks -= 50;
            if(floppy_motor_ticks <= 0) {
                floppy_motor_kill(floppy_base);
            }
        }
    }
}

// Seek for a given cylinder, with a given head
int floppy_seek(int base, unsigned cyli, int head) {

    unsigned i, st0, cyl = -1; // set to bogus cylinder

    floppy_motor(base, floppy_motor_on);

    for(i = 0; i < 10; i++) {
        // Attempt to position to given cylinder
        // 1st byte bit[1:0] = drive, bit[2] = head
        // 2nd byte is cylinder number
        floppy_write_cmd(base, CMD_SEEK);
        floppy_write_cmd(base, head<<2);
        floppy_write_cmd(base, cyli);
        tmp_sleep();
        //irq_wait(floppy_irq);
        floppy_check_interrupt(base, &st0, &cyl);

        if(st0 & 0xC0) {
            static const char * status[] =
            { "normal", "error", "invalid", "drive" };
            printk("floppy_seek: status = %s\n", status[st0 >> 6]);
            continue;
        }

        if(cyl == cyli) {
            floppy_motor(base, floppy_motor_off);
            return 0;
        }

    }

    printk("floppy_seek: 10 retries exhausted\n");
    floppy_motor(base, floppy_motor_off);
    return -1;
}
// Used by floppy_dma_init and floppy_do_track to specify direction
typedef enum {
    floppy_dir_read = 1,
    floppy_dir_write = 2
} floppy_dir;


// we statically reserve a totally uncomprehensive amount of memory
// must be large enough for whatever DMA transfer we might desire
// and must not cross 64k borders so easiest thing is to align it
// to 2^N boundary at least as big as the block
#define floppy_dmalen 0x4800
//static const char floppy_dmabuf[floppy_dmalen]
//                  __attribute__((aligned(0x8000)));
static uint32_t floppy_dmabuf;

static void floppy_dma_init(floppy_dir dir) {

    union {
        unsigned char b[4]; // 4 bytes
        unsigned long l;    // 1 long = 32-bit
    } a, c; // address and count

    a.l = (unsigned) floppy_dmabuf;
    c.l = (unsigned) floppy_dmalen - 1; // -1 because of DMA counting
    printk("dma_buf:%x\n", floppy_dmabuf);
    // check that address is at most 24-bits (under 16MB)
    // check that count is at most 16-bits (DMA limit)
    // check that if we add count and address we don't get a carry
    // (DMA can't deal with such a carry, this is the 64k boundary limit)
    if((a.l >> 24) || (c.l >> 16) || (((a.l&0xffff)+c.l)>>16)) {
        printk("floppy_dma_init: static buffer problem\n");
    }

    unsigned char mode;
    switch(dir) {
        // 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
        case floppy_dir_read:  mode = 0x46; break;
        // 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2
        case floppy_dir_write: mode = 0x4a; break;
        default: printk("floppy_dma_init: invalid direction");
                 return; // not reached, please "mode user uninitialized"
    }

    outb(0x0a, 0x06);   // mask chan 2

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x04, a.b[0]); //  - address low byte
    outb(0x04, a.b[1]); //  - address high byte

    outb(0x81, a.b[2]); // external page register

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x05, c.b[0]); //  - count low byte
    outb(0x05, c.b[1]); //  - count high byte

    outb(0x0b, mode);   // set mode (see above)

    outb(0x0a, 0x02);   // unmask chan 2
}

// This monster does full cylinder (both tracks) transfer to
// the specified direction (since the difference is small).
//
// It retries (a lot of times) on all errors except write-protection
// which is normally caused by mechanical switch on the disk.
//
int floppy_do_track(int base, unsigned cyl, floppy_dir dir) {

    // transfer command, set below
    unsigned char cmd;
    int i;
    // Read is MT:MF:SK:0:0:1:1:0, write MT:MF:0:0:1:0:1
    // where MT = multitrack, MF = MFM mode, SK = skip deleted
    //
    // Specify multitrack and MFM mode
    static const int flags = 0xC0;
    switch(dir) {
        case floppy_dir_read:
            cmd = CMD_READ_DATA | flags;
            break;
        case floppy_dir_write:
            cmd = CMD_WRITE_DATA | flags;
            break;
        default:

            printk("floppy_do_track: invalid direction");
            return 0; // not reached, but pleases "cmd used uninitialized"
    }

    // seek both heads
    if(floppy_seek(base, cyl, 0)) return -1;
    if(floppy_seek(base, cyl, 1)) return -1;

    for(i = 0; i < 20; i++) {
        floppy_motor(base, floppy_motor_on);

        // init dma..
        floppy_dma_init(dir);
        tmp_sleep();
        //timer_sleep(10); // give some time (100ms) to settle after the seeks

        floppy_write_cmd(base, cmd);  // set above for current direction
        floppy_write_cmd(base, 0);    // 0:0:0:0:0:HD:US1:US0 = head and drive
        floppy_write_cmd(base, cyl);  // cylinder
        floppy_write_cmd(base, 0);    // first head (should match with above)
        floppy_write_cmd(base, 1);    // first sector, strangely counts from 1
        floppy_write_cmd(base, 2);    // bytes/sector, 128*2^x (x=2 -> 512)
        floppy_write_cmd(base, 18);   // number of tracks to operate on
        floppy_write_cmd(base, 0x1b); // GAP3 length, 27 is default for 3.5"
        floppy_write_cmd(base, 0xff); // data length (0xff if B/S != 0)
        
        tmp_sleep();
        //irq_wait(floppy_irq); // don't SENSE_INTERRUPT here!

        // first read status information
        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        st0 = floppy_read_data(base);
        st1 = floppy_read_data(base);
        st2 = floppy_read_data(base);
        /*
         * These are cylinder/head/sector values, updated with some
         * rather bizarre logic, that I would like to understand.
         *
         */
        rcy = floppy_read_data(base);
        rhe = floppy_read_data(base);
        rse = floppy_read_data(base);
        // bytes per sector, should be what we programmed in
        bps = floppy_read_data(base);
        
        printk("st0:%x st1:%x st2:%x rcy:%x rhe:%x rse:%x bps:%x\n", st0, st1, st2, rcy, rhe, rse, bps);

        int error = 0;

        if(st0 & 0xC0) {
            static const char * status[] =
            { 0, "error", "invalid command", "drive not ready" };
            printk("floppy_do_sector: status = %s\n", status[st0 >> 6]);
            error = 1;
        }
        if(st1 & 0x80) {
            printk("floppy_do_sector: end of cylinder\n");
            error = 1;
        }
        if(st0 & 0x08) {
            printk("floppy_do_sector: drive not ready\n");
            error = 1;
        }
        if(st1 & 0x20) {
            printk("floppy_do_sector: CRC error\n");
            error = 1;
        }
        if(st1 & 0x10) {
            printk("floppy_do_sector: controller timeout\n");
            error = 1;
        }
        if(st1 & 0x04) {
            printk("floppy_do_sector: no data found\n");
            error = 1;
        }
        if((st1|st2) & 0x01) {
            printk("floppy_do_sector: no address mark found\n");
            error = 1;
        }
        if(st2 & 0x40) {
            printk("floppy_do_sector: deleted address mark\n");
            error = 1;
        }
        if(st2 & 0x20) {
            printk("floppy_do_sector: CRC error in data\n");
            error = 1;
        }
        if(st2 & 0x10) {
            printk("floppy_do_sector: wrong cylinder\n");
            error = 1;
        }
        if(st2 & 0x04) {
            printk("floppy_do_sector: uPD765 sector not found\n");
            error = 1;
        }
        if(st2 & 0x02) {
            printk("floppy_do_sector: bad cylinder\n");
            error = 1;
        }
        if(bps != 0x2) {
            printk("floppy_do_sector: wanted 512B/sector, got %d", (1<<(bps+7)));
            error = 1;
        }
        if(st1 & 0x02) {
            printk("floppy_do_sector: not writable\n");
            error = 2;
        }

        if(!error) {
            floppy_motor(base, floppy_motor_off);
            return 0;
        }
        if(error > 1) {
            printk("floppy_do_sector: not retrying..\n");
            floppy_motor(base, floppy_motor_off);
            return -2;
        }
    }

    printk("floppy_do_sector: 20 retries exhausted\n");
    floppy_motor(base, floppy_motor_off);
    return -1;

}

int floppy_read_track(int base, unsigned cyl) {
    return floppy_do_track(base, cyl, floppy_dir_read);
}

int floppy_write_track(int base, unsigned cyl) {
    return floppy_do_track(base, cyl, floppy_dir_write);
}

extern struct raw_page *kalloc_pa();
char *dma_va_buf;
void create_dma_mem()
{
    kmalloc_page(); //4

    kmalloc_page(); // 5
    kmalloc_page(); //6
    kmalloc_page(); //7
    dma_va_buf = kmalloc_page(); // 0x428000
    //uint32_t real_pa = unmap_page(dma_va_buf);
    uint32_t pa = 0x428000;// kalloc_pa();
    printk("dma:%x\n",pa);
    //pa = pa + 0x8000 - (pa % 0x8000);
    floppy_dmabuf = pa;
    printk("dma:%x\n",pa);
}

void init_dev()
{
    create_dma_mem();
    detect_floppy();
    floppy_reset(floppy_base);

    // for test
    char *buf = dma_va_buf;
    floppy_read_track(floppy_base, 1);
    printk("buf:%x,%x\n", buf[0], buf[1]);
    buf[0] = 0x32;
    buf[1] = 0x54;
    floppy_write_track(floppy_base, 1);
    tmp_sleep();
    buf[0] = 0;
    buf[1] = 0;
    printk("buf:%x,%x\n", buf[0], buf[1]);
    floppy_read_track(floppy_base, 1);
    tmp_sleep();
    printk("buf:%x,%x\n", buf[0], buf[1]);

}
