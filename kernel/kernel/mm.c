#include <string.h>
#include <stddef.h>
#include <multiboot.h>

int get_phynamic_memory_size()
{
    int t = memcpy();
    return t;
}

multiboot_info_t* g_mbd = NULL;
unsigned int g_magic = 0;
int init_memory(multiboot_info_t* mbd, unsigned int magic)
{
    if (magic != 0x2badb002) {
        return -1;
    }
    
	g_mbd = mbd;
	g_magic = magic;
    return 0;
}
