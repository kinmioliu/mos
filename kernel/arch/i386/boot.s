.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */
 
/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .data.multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
 
/*
The multiboot standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .bss, "aw", @nobits
.align 4096
/*.global boot_page_directory:
.type boot_page_directory, @DB
*/
boot_page_directory:
.skip 4096
boot_page_table1:
.skip 4096

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .text.multiboot
.global _start
.type _start, @function
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the
	machine.
	*/
	/*set pt*/
    /*
    * how this running
    */
    /* because boot_page_table is max then 0xC0000000, and currently the paging is not effect, so wei should minus 0xC0000000, get the pa of pt */
	movl $(boot_page_table1 - 0xC0000000), %edi
	movl $0, %esi
1:
	cmpl $kernel_start, %esi
	jl 2f
	cmpl $(kernel_end - 0xC0000000), %esi
	jge 3f
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	addl $4096, %esi
	addl $4, %edi
	loop 1b

    /*at this point, mapped phyaddr(kernel_start-kernel_end) to tbl1*/
3:	
    movl $(boot_page_table1 -0xC0000000 + 0x3), %edi
    movl %edi, (boot_page_directory - 0xC0000000 + 0) // set pt1 to 0, because at this time, we need a tmp low half kernel
    movl %edi, (boot_page_directory -0xC0000000 + 768 * 4) /*set page table1 to pd inx 768 is 0xC0000000*/    
    /* map vgb to 0xC03FFFFF set vgb to pt*/	
    movl $(0xB8000 + 0x3), %edx
    movl %edx, boot_page_table1 - 0xC0000000 + 1023 * 4 /* pd inx = 0xC03FF000 >> 22; pt_inx = 0xC03FF000 >> 12 & 0x3FF */
	/*enable paging*/

    /* vga and kernel is 768 index */
    
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3
	movl %cr0, %ecx
	orl $0x80000000, %ecx
	movl %ecx, %cr0
    
    
    /* at this point, paging is go into effect, and we have maped the kernel to 0xC0100000, which pa is 0x100000
    ** now we need jump to the vm */
    lea kernel, %ecx
    jmp *%ecx
 
	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
.section .text
kernel:
    // at this time , we are in high half kernel, so we disable the low half kernel
    movl $0, boot_page_directory - 0xC0000000 + 0    

    movl %cr3, %ecx
    movl %ecx, %cr3
	mov $stack_top, %esp
    /*
    dbg the pd    
    movl $(boot_page_directory) , %eax	
    movl $(boot_page_directory + 768 *4), %ebx
    */
    push %eax /*magic*/
    movl %ebx, %ecx
    addl $0xC0000000, %ecx
    movl %ecx, %ebx
	push %ebx /*mbd*/
    
    call init_memory
    call register_gdt
    call reload_segment
	/*
	This is a good place to initialize crucial processor state before the
	high-level kernel is entered. It's best to minimize the early
	environment where crucial features are offline. Note that the
	processor is not fully initialized yet: Features such as floating
	point instructions and instruction set extensions are not initialized
	yet. The GDT should be loaded here. Paging should be enabled here.
	C++ features such as global constructors and exceptions will require
	runtime support to work as well.
	*/
 	call _init 
	/*
	Enter the high-level kernel. The ABI requires the stack is 16-byte
	aligned at the time of the call instruction (which afterwards pushes
	the return pointer of size 4 bytes). The stack was originally 16-byte
	aligned above and we've pushed a multiple of 16 bytes to the
	stack since (pushed 0 bytes so far), so the alignment has thus been
	preserved and the call is well defined.
	*/
	call kernel_main

 	call _fini
	/*
	If the system has nothing more to do, put the computer into an
	infinite loop. To do that:
	1) Disable interrupts with cli (clear interrupt enable in eflags).
	   They are already disabled by the bootloader, so this is not needed.
	   Mind that you might later enable interrupts and return from
	   kernel_main (which is sort of nonsensical to do).
	2) Wait for the next interrupt to arrive with hlt (halt instruction).
	   Since they are disabled, this will lock up the computer.
	3) Jump to the hlt instruction if it ever wakes up due to a
	   non-maskable interrupt occurring or due to system management mode.
	*/
	cli
5:	hlt
	jmp 5b
 
/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
//.size _start, . - _start
