.section .text
.global ._start 
.type _start, @function
_start:
mov $1, %eax

.section .data
.long 0x90909090
