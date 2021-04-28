.section .data
ir0:
.byte 0xe0, 0x29, 0x8, 0x0,  0, 0x8E, 0x10, 0xC0
isr:
.skip 2048; #decal a 255*8 array
/*size(2) idtpointer*/
IDTR:
.byte 0xff, 0x7, 0, 0, 0, 0

.section .text
.type irq0, @function
irq0:
    pop %eax
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    cli
    push %eax
    #push $6
    #call keyboard_handler
    call page_fault_handler
    pop %eax
    #pop %eax
    #call dividezero_handler
    
    sti
    
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
#push $0
    iret

.global set_idt
.type set_idt, @function
set_idt:
    push %ebp
    movl %esp, %ebp
    push $0x12345678
    call print_asm_u32
create_isr:
    movl $(isr), %ebx 
fill_entry:
    movl $(irq0), %eax
    movb %al, (%ebx)    #1th
    movb %ah, 0x1(%ebx) #2th
    movw $0x08, 0x2(%ebx) #3,4th slector
    movb $0x0, 0x4(%ebx) #5th zero
    movb $0x8E, 0x5(%ebx) #6th zero
    shr $0x10, %eax
    movb %al, 0x6(%ebx) #7th 
    movb %ah, 0x7(%ebx) #8th 
    addl $0x8, %ebx #mov to next entry
    cmpl $(isr + 2048), %ebx # if ebx < isr+2040, then continue
    jl fill_entry
    push $(isr)
    call print_asm_u32
    #set ir0 again
    #movl $(irq0), %eax
    #and 0xffff, %eax
    movl $(isr), %eax
    movl %eax, (IDTR + 2)
    movl $(IDTR), %eax
    lidt (%eax)
    mov %ebp, %esp
    pop %ebp
    ret
