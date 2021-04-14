CC=i686-elf-gcc
AR=i686-elf-ar

DESTDIR=../sysroot/usr/local
SYSROOT = -isystem include/

CFLAGS?=-O2 -g

CFLAGS:=$(CFLAGS) -Wall -Wextra

BINARY=libc.a libk.a

SRCDIR = stdio string stdlib
ARCHDIR=arch/i386

SRCS = $(shell find $(SRCDIR) -type f -name '*.c')
OBJS = $(patsubst %.c, %.o, $(SRCS))

#OBJDIR=./objs

$(shell mkdir -p $(OBJDIR))

include $(ARCHDIR)/make.config

#OBJS=\
	$(ARCH_LIB_OBJS)\
	$(OBJDIR)/putchar.o \
	$(OBJDIR)/getchar.o \
	$(OBJDIR)/print.o \
	$(OBJDIR)/abort.o \
	$(OBJDIR)/memcpy.o \
	$(OBJDIR)/strlen.o \
	$(OBJDIR)/memset.o

#SRCS=\
	 $(ARCH_LIB_SRCS) \
	 stdio/putchar.c \
	 stdio/getchar.o \
	 stdio/print.c \
	 stdlib/abort.c \
	 string/memcpy.c \
	 string/strlen.c \
	 string/memset.c

.PHYON:all clean install_header install_lib

all:$(BINARY) install_header install_lib

$(BINARY):$(OBJS)
	$(AR) r $@ $(OBJS)

%.o:%.c
	$(CC) $(SYSROOT) $(CFLAGS) -c $< -o $@ -std=gnu99

install_header:
	mkdir -p $(DESTDIR)/include
	cp -rf include/* $(DESTDIR)/include/

install_lib:
	mkdir -p $(DESTDIR)/lib
	cp -rf libk.a $(DESTDIR)/lib/

clean:
	rm -rf $(OBJS)
	rm -rf $(BINARY)
