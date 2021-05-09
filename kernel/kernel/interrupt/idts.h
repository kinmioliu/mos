#ifndef __IDTS_H__
#define __IDTS_H__

#include <stdint.h>
typedef void* interrupt_frame_t;

void register_isr(uint16_t iid, uint32_t handler);

#endif
