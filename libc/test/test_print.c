#include <stdio.h>
extern int printk(const char* format, ...);

int main(int argc, char* argv[])
{
    printk("addr:%llx,len:%llx\n",0, 0x9fc00);   
    return 0;
}
