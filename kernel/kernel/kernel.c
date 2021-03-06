#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <idt.h>
#include <mm/mm.h>
#include <dev/dev.h>
#include <stdio.h>
#include <multiboot.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
extern multiboot_info_t* g_mbd;
extern unsigned int g_magic;
extern void print_boot_page_table();


size_t TAB_WIDTH = 8;

void __attribute__((constructor)) set_tab_width()
{
    TAB_WIDTH = 15;
}

/* Hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};
 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{	
    return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{	
    return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) 
{
    size_t len = 0;	
    while (str[len])	
        len++;		
    return len;
}

void terminal_memcpy(void *dest, void *src, size_t size)
{
    size_t inx = 0;
    while(inx < size) {
        *((char *)dest + inx) = *((char *)src + inx);
        inx++;
    }
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));
uint32_t pt_768[1024] __attribute__((aligned(4096)));

void init_terminal(void) 
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*)0xb8000;
    terminal_buffer = (uint16_t*)0xC03FF000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    printk("initialize terminal [OK]\n");
}
 
void terminal_setcolor(uint8_t color) 
{
    terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

extern void __attribute__((optimize("O0"))) tmp_sleep();
void __attribute__((optimize("O0"))) screen_scrolling_up(size_t lines)
{
    for(int i = 0; i < 10; i++) {
        tmp_sleep();
    }
    if (lines <= 0 || lines > VGA_HEIGHT) {
        return;
    }
#if 0
    /*
     * !!! can't use memcpy, because terminal_buffer is 2Byte, memcpy operate the buffer 
     * by 1Byte and it will affect the atomic of screen flash.(I guessed), 
     * */
    char *dest = (char *)0xB8000;
    char *src = (char *)0xB8000 + lines * VGA_WIDTH;
    memcpy(dest, src, sizeof(uint16_t) * (VGA_HEIGHT - lines) * VGA_WIDTH);
#endif
    size_t destInx = 0;
    size_t srcInx = lines * VGA_WIDTH;
    for (size_t i = 0; i < ((VGA_HEIGHT - lines) * VGA_WIDTH); i++) {
        terminal_buffer[destInx + i] = terminal_buffer[srcInx + i];
    }
    // init left lines
    for (size_t y = VGA_HEIGHT - lines; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++){
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = VGA_HEIGHT - 1;
}


static inline void put_newline_char()
{
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT){
        //terminal_row = 0;
        screen_scrolling_up(1);
    } 
}

static inline void put_tab_char()
{
    for (size_t i = 0; i < TAB_WIDTH; i++) {
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT){
                // terminal_row = 0;
                screen_scrolling_up(1);
            }
        }
    }
}

bool is_special_character(char c)
{
    return (c == '\n') || (c == '\t'); 
}

void terminal_putchar(char c) 
{
    if (!is_special_character(c)) {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {            
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT)        
                //terminal_row = 0;    
                screen_scrolling_up(1);
        }
        return;   
    }
    switch (c) {
        case '\n':
            put_newline_char();
            break;
        case '\t':
            put_tab_char();
            break;
        default:
            break;
    }
}
 
void terminal_write(const char* data, size_t size) 
{
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) 
{
    terminal_write(data, strlen(data));
}

void __attribute__((optimize("O0"))) tmp_sleep()
{
    float result;
    float a = 109.143;
    float b = 107.365;
    size_t i = 0;
    while (i < 1000000) {
        //result = a * b;
        i++;
    }
}

int rand(void) {        
    static int next = 1;
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
}


char get_random_char()
{
    if (rand() % 100 >= 95) {
        return '\t';
        return (rand() % 2) ? '\t' : '\n';
    }
    if (rand() % 2) {
        int num = rand() % 10;
        return '0' + num;
    }
    int c = rand() % 26 ;
    return rand() % 2 ? 'A' + c : 'a' + c;
}

void set_pd_not_present()
{
	int i;
	for(i = 0; i < 1024; i++)
	{
    // This sets the following flags to the pages:
    //   Supervisor: Only kernel-mode can access them
    //   Write Enabled: It can be both read from and written to
    //   Not Present: The page table is not present
    	page_directory[i] = 0x00000002;
	}
}


void set_pt1()
{
	unsigned int i = 0xb8;
	//we will fill all 1024 entries in the table, mapping 4 megabytes
	for(; i < 1024; i++)
	{
    	// As the address is page aligned, it will always leave 12 bits zeroed.
    	// Those bits are used by the attributes ;)
    	first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
	}
}

void set_pt768()
{
    unsigned int i = 0;
}

void __attribute__((optimize("O0"))) divide()
{
    int a = 56;
    int b = 0;
    int c = a/b;
}

// This should go outside any function..
extern void loadPageDirectory(unsigned int*);
extern void enablePaging();
extern void test_big_stack_frame();
extern volatile uint32_t g_pic_count;

void kernel_main(void) 
{
    /* Initialize terminal interface */
    int tmp;
    init_terminal();
    init_interrupt();
    init_mm();
    init_timer();
    init_sched();
    init_dev();
    volatile uint32_t last_g_pic_count = g_pic_count;
    // 600 cycles consume about 32s
    while(1) {
        if (g_pic_count != 0 && last_g_pic_count != g_pic_count && g_pic_count % 75 == 0) {
            printk("pit interrupt: %d\n", g_pic_count);
            last_g_pic_count = g_pic_count;
        }
    }
    printk("hlt\n");
    asm volatile ("hlt");
    //divide();
    return;
    
    printk("pd:%x, pt:%x\n", page_directory, first_page_table);
    PIC_remap(0x20, 0x28);
    print_boot_page_table();
    test_idts();
    printk("tmp:%x\n", &tmp);
    //print_multiboot_info();
    test_big_stack_frame();
        return;
    /* Newline support is left as an exercise. */
    terminal_writestring("Hello, kernel World!\nthis is a new line.\n");
    terminal_writestring("another str.\n");
    terminal_writestring("this is a tab\ta.\n");
    for (int i = 0; i < 30; i++) {
        terminal_writestring("this is a line");
        terminal_putchar('0'+i);
        terminal_putchar('\n');
        tmp_sleep();
    }
    // terminal_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_BLUE));
    printk("kernel_main\tend!:%x\n", get_phynamic_memory_size);
    
    if (get_phynamic_memory_size() != -1) {
        terminal_writestring("get_physical_memory_size()\n");
    }
    if (g_mbd == NULL) {
	    terminal_writestring("mdb is a a null\n");
    } else {
	    terminal_writestring("mdb not a a null\n");
    }

	printk("g_mbd is:0x%x, magic is:0x%x\n", g_mbd, g_magic);
    print_boot_page_table();
    test_big_stack_frame();
    return;
    printk("mem_upper:%x, mem_lower:%x, boot_device:%x, mmap_length:%x, mmap_addr:%x\n",
            g_mbd->mem_upper, g_mbd->mem_lower, g_mbd->boot_device, g_mbd->mmap_length, g_mbd->mmap_addr);
    printk("sizeof multiboot_mmap_entry:%d\n", sizeof(multiboot_memory_map_t));
    print_multiboot_info();


    if (g_magic == 0x534d4150 || g_mbd == 0x534d4150) {
    	terminal_writestring(" detected magic\n");
	if (g_magic == 0x534d4150) {
		terminal_writestring(" g_magic\n");
	} else {
		terminal_writestring(" gmbd\n");
	}
    } else {
	terminal_writestring("no magic\n");
    }

    /*
    terminal_row = VGA_HEIGHT - 2;
    terminal_column = 0;
    for (int i = 0; i < 77; i++)
        terminal_putchar('t');
    terminal_putchar('\t');
    terminal_putchar('t');
    terminal_putchar('\n');
    */

}
