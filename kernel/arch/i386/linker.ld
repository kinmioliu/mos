/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)
/*
SEARCH_DIR(./arch/i386/)
STARTUP(boot.o)
*/
/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
    . =0x100000 ;
    kernel_start = . ;
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	/*the multiboot name */
    /*.multiboot.data ABSOLUTE(0x1000) : AT(ADDR(.multiboot.data))*/
    .data.multiboot : ALIGN(4K)
	{
		*(.data.multiboot)
	}
    .text.multiboot : ALIGN(4K)
    {
        *(.text.multiboot)
    }

    . += 0xC0000000 ;   
    
    .text ALIGN(4K) :AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
    }
   
	/* Read-only data. */
	.rodata ALIGN(4K) :AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) :AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	/*.bss BLOCK(4K) : ALIGN(4K)*/
	.bss ALIGN(4K) :AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
	kernel_end = .;

	/* The compiler may produce other sections, put them in the proper place in
	   in this file, if you'd like to include them in the final kernel. */
}
