#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <multiboot.h>
multiboot_info_t* g_mbd = NULL;
unsigned int g_magic = 0;
int get_phynamic_memory_size()
{
    int t = memcpy();
    return t;
}

static inline uint32_t get_cr3()
{
    uint32_t cr3;
    __asm__ volatile (
        "movl %%cr3, %0;"
        :"=a"(cr3)
    );
    return cr3;
}

#if defined(__i386__)
/* i386 has a two level pt 
 * |31    22|21    12|11  0|
 *    pd       pt      page_off
**/
uint32_t *page_directory;
void set_page_directory()
{
    uint32_t cr3 = get_cr3();
    page_directory = (uint32_t *)(cr3 + 0xC0000000);
}

#define PD_INX(VA) ((VA) >> 22)
#define PT_INX(VA) ((VA) >> 12 & 0x3FF)
#define PT_TBL(VA) ((page_directory[PD_INX(VA)] & 0xFFFFF000) + 0xC0000000)
#define VA2PA(VA) (((uint32_t *)PT_TBL((VA)))[PT_INX(VA)] & 0xFFFFF000 + ((VA)&0xFFF) )
#endif


void print_boot_page_table()
{
    set_page_directory();
    unsigned long pd;
    __asm__ volatile (
        "movl %%cr3, %0;"
	:"=a"(pd)
	);
    printk("va:0x%x, pa:0x%x\n", 0xC03FF000, VA2PA(0xC03FF000));
    printk("cr3:0x%x\n", pd );
    unsigned int *pt1 = (unsigned long*)(((unsigned long*)(pd + 0xC0000000))[0xC0100000>>22] & 0xfffff000);
    printk("pt1:0x%x\n", pt1);
    pt1 = (unsigned int)pt1 + 0xC0000000;  // pt1 is a pa, so we should format it to va
    printk("val:%x\n", *pt1) ;
    for (int i = 0; i < 1024; i++){
        printk("%x:%x ",pt1+i, *(pt1 + i));
        if (i != 0 && i % 8 == 0) {
            printk("\n%x ", i);
        }
    }
    printk("\n");
}

void print_multiboot_info()
{
    printk("flags:%x\n", g_mbd->flags);
    if (g_mbd->flags & MULTIBOOT_INFO_MEMORY) {
        printk("mem_lower:0x%x, mem_upper:0x%x kb\n", g_mbd->mem_lower, g_mbd->mem_upper);        
    }
    if (g_mbd->flags & MULTIBOOT_INFO_MEM_MAP) {
        printk("mmap_length:0x%x, mmap_addr:0x%x\n", g_mbd->mmap_length, g_mbd->mmap_addr);
        multiboot_memory_map_t* entry = (multiboot_memory_map_t *)((uint32_t)(g_mbd->mmap_addr) + 0xC0000000);
        while (entry < g_mbd->mmap_addr + g_mbd->mmap_length) {
            printk("size:%x, addr:%llx, len:%llx, type:%x\n", entry->size, entry->addr, entry->len, entry->type);
#if 0
            int i;
            // test visit a invalid memory
            for (i = 0; i < 100; i++) {             
                char *c = (char *)(entry->addr + entry->len - 50 + i);
                *c = 'a';
                printk("%d:", i);
                putchar(*c);
                putchar(' ');
            }           
            printk("\n");          
#endif            
            entry++; // (unsigned int)entry + entry->size + sizeof(entry->size)            
        }
       /*
       *(char *)(entry->addr + entry->len - 1) = 'a';
       *(char *)(entry->addr + entry->len) = 'b';
       putchar(*(char *)(entry->addr + entry->len - 1));
       putchar(*(char *)(entry->addr + entry->len));
       */
    }
}


int init_memory(multiboot_info_t* mbd, unsigned int magic)
{
    if (magic != 0x2badb002) {
        //return -1;
    }
    
	g_mbd = mbd;
	g_magic = magic;
    return 0;
}
