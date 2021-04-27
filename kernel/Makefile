
CC = i686-elf-gcc
AS = i686-elf-as

SYSROOT=-isystem ../sysroot/usr/local/include/ -isystem ./kernel/ -isystem ./include/ -L ../sysroot/usr/local/lib

#default configure
CFLAGS ?= -O2 -g # ?= if not assigned ,then assigned
LIBS ?=

#set additional flag to this Makefile
CFLAGS := $(CFLAGS) -ffreestanding -Wall -Wextra
LIBS := $(LIBS) -nostdlib -lk -lgcc #use libgcc.a libk.a

ARCHDIR=arch/i386
include $(ARCHDIR)/make.config

#append the arch flags
CFLAGS := $(CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBS := $(LIBS) $(KERNEL_ARCH_LIBS)

#objs
KERNEL_OBJS = \
	$(KERNEL_ARCH_OBJS) \
	kernel/debug.o \
	kernel/kernel.o \
	kernel/mm.o \
	kernel/idts.o \
	kernel/pic.o

OBJS = \
	$(ARCHDIR)/crti.o \
	$(ARCHDIR)/crtbegin.o \
	$(KERNEL_OBJS) \
	$(ARCHDIR)/crtend.o \
	$(ARCHDIR)/crtn.o

LINK_LIST = \
	$(LDFLAGS) \
	$(ARCHDIR)/crti.o \
	$(ARCHDIR)/crtbegin.o \
	$(KERNEL_OBJS) \
	$(ARCHDIR)/crtend.o \
	$(ARCHDIR)/crtn.o

all:mos.kernel

#because mos.kernel is a archive file, so $@ is mos.kernel, if target is object file, then $@ is multi files
mos.kernel:$(OBJS)
	$(CC) $(SYSROOT) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST) $(LIBS)
	grub-file --is-x86-multiboot mos.kernel

$(ARCHDIR)/crtbegin.o:
	cp $(shell $(CC) -print-file-name=crtbegin.o) $(ARCHDIR)/

$(ARCHDIR)/crtend.o:
	cp $(shell $(CC) -print-file-name=crtend.o) $(ARCHDIR)/

$(ARCHDIR)/%.o:$(ARCHDIR)/%.s
	$(AS) $< -o $@

kernel/%.o:kernel/%.c
	$(CC) $(SYSROOT) $(CFLAGS) -c $< -o $@ -std=gnu99

clean:
	rm -rf $(ARCHDIR)/*.o
	rm -rf kernel/*.o
	rm -rf mos.kernel

