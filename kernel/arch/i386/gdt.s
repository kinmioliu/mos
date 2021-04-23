.section .data
/* descriptor structure:
* base limit flags accessbyte
* we set all segment coverage 4G address space
*/
GDT_content:
.byte 0, 0, 0, 0, 0, 0, 0, 0 # selector 0x0 this is descriptor 0
.byte 0xff, 0xff, 0, 0, 0, 0x9A, 0xCF, 0 # selector 0x8 descriptor 1 for KM code segment
.byte 0xff, 0xff, 0, 0, 0, 0x92, 0xCF, 0 # selector 0x10 descriptor 2 for KM data segment
.byte 0xff, 0xff, 0, 0, 0, 0xFA, 0xCF, 0 # selector 0x18  descriptor 3 for UM code segment
.byte 0xff, 0xff, 0, 0, 0, 0xF2, 0xCF, 0 # selector 0x20 descriptor 4 for UM data segment
#.byte 0x67, 0, 0, 0, 0, 0xE9, 0, 0 # selector 0x28 descriptor 5 for Task switch segment

/* size| entry addr */
GDTR:
.byte 39, 0, 0, 0 , 0, 0

.section .text
.global register_gdt
.type registrer_gdt, @function
register_gdt:
    cli; # close interrupt
    movl $(GDT_content), GDTR + 2
    movl $(GDTR), %eax
    lgdt (%eax)
    movl %cr0, %eax
    or $1, %eax # enable Protected Mode
    movl %eax, %cr0
    ret

/* data segment can be modified by mov instruction(mov ax, ds), but cs
so we use jmp to lode code segment selector*/
.section .text
.global reload_segment
.type reload_segment, @function
reload_segment:
    jmp $0x8,$reload_cs /*0x8 points at code selector, the cs will 0x8 and ip(instruction pointer) is reload_cs*/
reload_cs:
    mov $0x10, %ax /*0x10 is a sector 0b 10(inx) 0(gdt) 00(priv) witch points at data sector*/
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret
