#include <stddef.h>
extern void terminal_putchar(char c);
int putchar(char c)
{
    terminal_putchar(c);
    return 0;
}
