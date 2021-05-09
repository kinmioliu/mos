#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

void PIC_remap(int offset1, int offset2);
void PIC_sendEOI(unsigned char irq);

#endif


