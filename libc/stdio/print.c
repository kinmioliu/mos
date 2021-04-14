#include <stdio.h>
#include <stdarg.h>
static const char ERROR_DIGIT_CHAR = '@';
static inline char hex_digit_char(unsigned int i)
{
    if (i > 0xf) {
        return ERROR_DIGIT_CHAR;
    }
    return i >= 10 ? 'a' + i - 10 : '0' + i;
}

/* 
 * 32bit
 * */
static void print_int(int integer)
{
    int istrlen = 0;
    int inx = 0;
    int remain;
    char istr[10];
    if (integer == 0) {
        putchar('0');
        return;
    }
    // can't convert negative to postive integer, conside about minint 
    if (integer < 0) {
        putchar('-');
    }
#define digitc(i) (i) > 0 ? '0' + (i) : '0' - (i)
    while(integer) {
        remain = integer % 10;
        istr[istrlen++] = digitc(remain);
        integer /= 10;
    }
    for (; inx < istrlen; inx++) {
        putchar(istr[istrlen - 1 - inx]);
    }
}

static void print_uint(unsigned int ui)
{
}

static void print_uhex(unsigned int uinteger)
{
    int uistrlen = 0;
    int inx = 0;
    unsigned int remain;
    char str[8];

    if(uinteger == 0) {
        putchar('0');
        return;
    }
    while (uinteger) {
        remain = uinteger & 0xf;
        str[uistrlen++] = hex_digit_char(remain);
        uinteger >>= 4;
    }
    for (; inx < uistrlen; inx++) {
        putchar(str[uistrlen - 1 - inx]);
    }    
}

/*
 * usage: 
 * printk("hello world");
 * printk("num %d", a);
 * ...
 * */
int printk(const char* format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    // travel format, if meet %x, then get a argument
    const char* c = format;
    int integer;
    unsigned uinteger;
    while (*c != 0) {
        if (*c == '%') {
            //get and matcher next char
            switch(*(++c)) {
                case 'd':
                    integer = va_arg(arg_ptr, int);
                    //va_end(arg_ptr);
                    print_int(integer);
                    break;
                case 'x':
                    uinteger = va_arg(arg_ptr, unsigned int);
                    //va_end(arg_ptr);
                    print_uhex(uinteger);
                    break;
                default:
                    putchar('%');
                    putchar(*c);
                    break;
            }
        } else {
            putchar(*c);
        }
        c++; // get next character
    }
    va_end(arg_ptr);
    return 0;
}
