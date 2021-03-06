#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <multiboot.h>
#include <idts.h>
#include <list.h>
#include <idt.h>

multiboot_info_t* g_mbd = NULL;
multiboot_memory_map_t phymem_map; // TODO:current is the biggest memory region, it should use a array to hold all map


/*
 *
 * how we access pt by virtual address, if we want access 5th pt, we just access (uint32_t *)( 0xFFC00000 + 5 * 4096),
 * actually, the cpu will process by 0xFFC05000 get the pd_inx 1023 and pt_inx 5
 * pd[1023] is pt_meta, and pt_meta[5] is the pt 's phyaddress
 * 
 * */

unsigned int g_magic = 0;
int get_phynamic_memory_size()
{
    int t = memcpy();
    int m = memcpy();
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

uint32_t pt_meta[1024] __attribute__((aligned(4096)));
uint32_t pt_begin = 0xFFC00000;

void load_pt_meta()
{
    page_directory[1023] = ((uint32_t)pt_meta - 0xC0000000) + 3;
}

void load_pt(uint32_t id, uint32_t pa)
{
    pt_meta[id] = pa + 3;
}

void print_boot_page_table()
{
    set_page_directory();
    return;
    unsigned long pd;
    __asm__ volatile (
        "movl %%cr3, %0;"
	:"=a"(pd)
	);
    printk("va:0x%x, pa:0x%x\n", 0xC03FF000, VA2PA(0xC03FF000));
    printk("cr3:0x%x\n", pd);
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
    uint32_t best_regin = 0;
    printk("flags:%x\n", g_mbd->flags);
    if (g_mbd->flags & MULTIBOOT_INFO_MEMORY) {
        printk("mem_lower:0x%x, mem_upper:0x%x kb\n", g_mbd->mem_lower, g_mbd->mem_upper);        
    }
    if (g_mbd->flags & MULTIBOOT_INFO_MEM_MAP) {
        printk("mmap_length:0x%x, mmap_addr:0x%x\n", g_mbd->mmap_length, g_mbd->mmap_addr);
        multiboot_memory_map_t* entry = (multiboot_memory_map_t *)((uint32_t)(g_mbd->mmap_addr) + 0xC0000000); // trans to va

        while (entry < g_mbd->mmap_addr + g_mbd->mmap_length + 0xC0000000) {
            printk("size:%x, addr:%llx, len:%llx, type:%x\n", entry->size, entry->addr, entry->len, entry->type);
            if (entry->type == 1 && best_regin < entry->len)  {
                phymem_map = *entry;
                best_regin = entry->len;
            }
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

uint32_t pt_xxx[1024] __attribute__((aligned(4096)));

__attribute__ ((interrupt)) void page_fault_handler(interrupt_frame_t frame)
{
    struct Test{
        uint32_t err;
    } *test = frame;
    
    uint32_t mem;
    __asm__ ("movl %%CR2, %0"
            :"=b"(mem));
    printk("pagefault, err:%x, mem:%x\n", test->err, mem);

    uint32_t pd_inx = mem >> 22;
    uint32_t pt_inx = mem >> 12 & 0x3FF;
/*
    printk("pd:%x,pt:%x,%d\n", pd_inx, pt_inx, ((uint32_t *)PT_TBL(mem))[pt_inx]);
    uint32_t flags = ((uint32_t *)PT_TBL(mem))[pt_inx] & 0xFFF;
    printk("flags:%x\n", flags);
*/    
    printk("page_directory:%x\n", page_directory[pd_inx]);
    
    if ((page_directory[pd_inx] & 0x1) == 0) {
        printk("null pd\n");
    }    
    //((uint32_t *)PT_TBL(mem))[pt_inx] = g_mdb + (flags | 0x1);
    //pt_xxx[pt_inx] = 0x100000+3;
    printk("pd:%x,pt:%x\n", pd_inx, pt_inx);
    cli();
}

void init_pagefault()
{
    register_isr(14, (uint32_t)page_fault_handler);
}

int init_memory(multiboot_info_t* mbd, unsigned int magic)
{
    if (magic != 0x2badb002) {
        return -1;
    }
    
    g_mbd = mbd;
    g_magic = magic;
    return 0;
}

struct raw_page {
 //   struct list_head link;
 //   uint32_t flags;
 //   uint32_t va;
    char page[0];
};

const uint32_t PAGE_SIZE = 4096;
const uint32_t RAW_PAGE_SIZE = sizeof(struct raw_page) + PAGE_SIZE;
const uint32_t KERNEL_PHY_SIZE = 0x300000;  // left kernel 3M
const uint32_t PA_BM_VA = 0xC0300000;
const uint32_t VA_BM_VA = PA_BM_VA + PAGE_SIZE;
uint32_t pa_begin; 
uint32_t pmm_bm_size;

#define BM_CNT_PER_PAGE (8 * PAGE_SIZE)

struct VaObj {
    struct list_head used;
    struct list_head free;
    uint32_t usedCnt;
    uint32_t freeCnt;
    char *bm;
};

struct Allocator {
    uint8_t *bm;
    uint32_t size;
    uint32_t cursor;
};

struct Allocator kpa_allocator;
struct Allocator kva_allocator;

bool bm_test(uint8_t *bm, uint32_t pos)
{
    return bm[pos / 8] & (1 << (pos % 8));
}

void bm_set(uint8_t *bm, uint32_t pos)
{
    bm[pos / 8] |= (1 << (pos % 8));
}

void bm_clear(uint8_t *bm, uint32_t pos)
{
    bm[pos / 8] &= ~(1 << (pos % 8));
}

uint32_t get_pa_by_mmap(uint32_t inx)
{
    return pa_begin + inx * RAW_PAGE_SIZE;
}

uint32_t get_pmid_by_pa(uint32_t pa)
{
    return (pa - pa_begin) / RAW_PAGE_SIZE;
}


struct VaPage {
    uint32_t va;
    uint32_t pa;
    struct list_head link;
};

struct VaObj vaObj;

struct raw_page *kalloc_pa()
{
    // search bm 
    uint32_t pmid = INVALID_VALUE32;
    uint32_t id = kpa_allocator.cursor;
    for (; id < kpa_allocator.size; id++) {
        if (!bm_test(kpa_allocator.bm, id)) {
            kpa_allocator.cursor = id;
            bm_set(kpa_allocator.bm, id);
            pmid = id;
            break;
        }
    }
    if (id == kpa_allocator.size) {
        for (id = 0; id < kpa_allocator.cursor; id++) {
            if (!bm_test(kpa_allocator.bm, id)) {
                kpa_allocator.cursor = id;
                bm_set(kpa_allocator.bm, id);
                pmid = id;
                break;
            }
        }
    }
    if (pmid == INVALID_VALUE32) {
        printk("no memory\n");
    }
    // printk("kalloc_pa id:%x\n", pmid);
    struct raw_page *ppage = get_pa_by_mmap(pmid);
    return ppage;
}

void kfree_pa(uint32_t pa)
{
    uint32_t pos = get_pmid_by_pa(pa);
    bm_clear(kpa_allocator.bm, pos);
}


void mmap_page(uint32_t va, uint32_t pa)
{
    uint32_t pdinx = va >> 22;
    uint32_t ptinx = va >> 12 & 0x3ff;
    if ((page_directory[pdinx] & 0x1) == 0) {
        struct raw_page *page = kalloc_pa();
        page_directory[pdinx] = (uint32_t)page + 3; // 0xC0000000 + (uint32_t)page + 3; // allocate a new tlb
        load_pt(pdinx, page);
    }
    if (va < 0xC0000000) {
        uint32_t *pt = pt_begin + pdinx * 0x1000;
        pt[ptinx] = pa + 3;
        return;
    }

    // we must map a tlb to va
    ((uint32_t *)PT_TBL(va))[ptinx] = pa + 3;
}

uint32_t unmap_page(uint32_t va)
{
    uint32_t pdinx = va >> 22;
    uint32_t ptinx = va >> 12 & 0x3ff;
    uint32_t pa = 0;
    if ((page_directory[pdinx] & 0x1) == 0) {
        printk("unmap a invalid page %x\n", va);
        return pa;
    }
    if (va < 0xC0000000) {
        uint32_t *pt = pt_begin + pdinx * 0x1000;
        pa = pt[ptinx] & 0xFFFFF000;
        pt[ptinx] = 0; // umap;
    } else {
        pa = ((uint32_t*)PT_TBL(va))[ptinx];
        ((uint32_t*)PT_TBL(va))[ptinx] = 0; // umap
    }
    // flush tlb
    __asm__ volatile(
        "movl %cr3, %ecx\t\n"
        "movl %ecx, %cr3\t\n");
   return pa;
}

uint32_t set_used(uint32_t id)
{
    return 0;
}

uint32_t kva_allocate()
{
    uint32_t id = kva_allocator.cursor;
    uint32_t vaid = INVALID_VALUE32;
    for (; id < kva_allocator.size; id++) {
        if (!test_va_bm(id)) {            
            kva_allocator.cursor = id;
            set_va_bm(id);
            vaid = id;
            break;
        }
    }
    if (id == kva_allocator.size) {
        for (id = 0; id < kva_allocator.cursor; id++) {
            if (!test_va_bm(id)) {
                kva_allocator.cursor = id;
                set_va_bm(id);
                vaid = id;
                break;
            }
        }
    }
    if (vaid == INVALID_VALUE32) {
        printk("no va memory\n");
    }
    //printk("kva_allocate id:%x\n", vaid);
    return vaid * PAGE_SIZE;
}

void kva_free(uint32_t va)
{
    uint32_t id = va / PAGE_SIZE;
    if (!test_va_bm(id)) {
        printk("free valid memory %x !\n", va);
        return;
    }
    clear_va_bm(id);
    kva_allocator.cursor = id;
}

void* kmalloc_page()
{
    uint32_t va = kva_allocate();
    if (va == 0) {
        // no memory
        return NULL;
    }
    struct raw_page *pa_page = kalloc_pa();
    if (pa_page == NULL) {
        // TODO: try swap some page to disk
        kva_free(va);
        return NULL;
    }
    mmap_page(va, pa_page);
    return va;
}

void kfree_page(void *va)
{
    //1. get vpage 
    kva_free(va);
    uint32_t pa = unmap_page(va);
    printk("free pa:%x\n", pa);
    kfree_pa(pa);
}

void init_phy_allocator()
{
    pa_begin = phymem_map.addr + KERNEL_PHY_SIZE;
    
    uint32_t availSize = phymem_map.len - pa_begin;
    // set used map
    uint32_t pa_raw_pgs = (availSize + RAW_PAGE_SIZE - 1) / RAW_PAGE_SIZE;
    uint32_t pa_bm_size = (pa_raw_pgs + 7) / 8;
    if (pa_bm_size > PAGE_SIZE) {
        printk("pa bm %d exceed one page limits \n", pa_bm_size);
    }
    uint32_t pa_bm = pa_begin;
    mmap_page(PA_BM_VA, pa_begin);
    kpa_allocator.bm = PA_BM_VA;
    kpa_allocator.size = pa_raw_pgs;
    kpa_allocator.cursor = 0;
    bm_set(kpa_allocator.bm, 0);
    printk("row_page_size:%x, pa_bm_va:%x, pa_begin:%x\n", RAW_PAGE_SIZE, PA_BM_VA, pa_begin);
}

void set_va_bm(uint32_t id)
{
    //TODO: check va align to PAGE
    uint32_t va_bm_id = id / BM_CNT_PER_PAGE;
    uint32_t bm_id = id % BM_CNT_PER_PAGE;
    uint8_t *bm_page = (uint8_t *)(((uint32_t *)kva_allocator.bm)[va_bm_id]);
    bm_set(bm_page, bm_id);
}

void clear_va_bm(uint32_t id)
{ 
    //TODO: check va align to PAGE
    uint32_t va_bm_id = id / BM_CNT_PER_PAGE;
    uint32_t bm_id = id % BM_CNT_PER_PAGE;
    uint8_t *bm_page = (uint8_t *)(((uint32_t *)kva_allocator.bm)[va_bm_id]);
    bm_clear(bm_page, bm_id);
}

bool test_va_bm(uint32_t id)
{
    uint32_t va_bm_id = id / BM_CNT_PER_PAGE;
    uint32_t bm_id = id % BM_CNT_PER_PAGE;
    uint8_t *bm_page = (uint8_t *)(((uint32_t *)kva_allocator.bm)[va_bm_id]);
    return bm_test(bm_page, bm_id);
}

void init_va_allocater(uint32_t page_cnt)
{
    uint32_t bm_pgs = (page_cnt + (PAGE_SIZE * 8) - 1) / (PAGE_SIZE * 8);
    printk("va page_cnt:%x, bm_pgs:%x\n", page_cnt, bm_pgs);
    struct raw_page *root_page = kalloc_pa();
    printk("root_page:%x\n", root_page);
    mmap_page(VA_BM_VA, root_page);
    kva_allocator.bm = VA_BM_VA;
    kva_allocator.size = page_cnt;
    int i;
    for (i = 0; i < bm_pgs; i++) {
        struct raw_page *va_bm_pg = kalloc_pa();
        mmap_page(VA_BM_VA + i * RAW_PAGE_SIZE, va_bm_pg->page);
        ((uint32_t *)VA_BM_VA)[i] = VA_BM_VA + (i + 1) * RAW_PAGE_SIZE;
    }
    // set va bm used
    set_va_bm(VA_BM_VA / PAGE_SIZE);
    for (i = 0; i < bm_pgs; i++) {
        set_va_bm((VA_BM_VA + i * RAW_PAGE_SIZE) / PAGE_SIZE);
    }
    set_va_bm(0); // address 0 is unvailable
    kva_allocator.cursor = 0;
    return;
}

void print_pa_bm()
{
    printk("++++++++++++++++++++++kpa_allocator+++++++++++++++++++++++++++\n");
    printk("bm:0x%x, cursor:%d, size:%d pages\n", kpa_allocator.bm, kpa_allocator.cursor, kpa_allocator.size);
    for (int i = 0; i < kpa_allocator.size; i++) {
        if (bm_test(kpa_allocator.bm, i)) {
            printk("%d ", i);
        }
    }
    printk("\n");
    printk("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

void print_va_bm()
{ 
    printk("++++++++++++++++++++++kva_allocator+++++++++++++++++++++++++++\n");
    printk("bm:0x%x, cursor:%d, size:%d pages\n", kva_allocator.bm, kva_allocator.cursor, kva_allocator.size);
    for (uint64_t va = 0xC0000000; va < 0xFFFFFFFF; va += PAGE_SIZE) {
        if(test_va_bm(va / PAGE_SIZE)) {
            printk("0x%x ", va);
        }
    }
    printk("\n");
    printk("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

void init_vaobj()
{   
    init_phy_allocator();
    vaObj.usedCnt = 0;    
    vaObj.freeCnt = 0;
    init_va_allocater(4*1024*1024/4);
}

void init_mm()
{
    printk("init memory.\n");
    printk("mbd:%x, magic:%x\n", g_mbd, g_magic);
    set_page_directory();
    load_pt_meta();
    printk("pd_va:%x,pd_pa:%x\n", page_directory, VA2PA((uint32_t)page_directory));
    print_multiboot_info();
    init_vaobj();
    // print_pa_bm();
    // print_va_bm();
    printk("raw page size:%d\n", RAW_PAGE_SIZE);
    char *new_va_page = kmalloc_page();
    printk("alloc new page:%x\n", new_va_page);
    new_va_page[0] = 'h';
    new_va_page[1] = 'e';
    new_va_page[2] = 'l';
    new_va_page[3] = 'l';
    new_va_page[4] = 'o';
    new_va_page[5] = '\0';
    printk("page content:%s\n", new_va_page);
    kfree_page(new_va_page);
    printk("init memory success.\n");
}
