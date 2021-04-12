OBJS:=boot.o kernel.o
TARGET=mos.bin

#-w disable all warning -Wall print all warning -Wextra other warning expect all, such as ==
CFLAGS := -ffreestanding -Wall -Wextra

CC:=i686-elf-gcc
AS:=i686-elf-as

OBJ_RTBEGIN:=$(shell $(CC) -print-file-name=crtbegin.o)
OBJ_RTEND:=$(shell $(CC) -print-file-name=crtend.o)

OBJ_LINK:=crti.o $(OBJ_RTBEGIN) $(OBJS) $(OBJ_RTEND) crtn.o


all:$(OBJS) crti.o crtn.o
	$(CC) -T linker.ld -o $(TARGET) -ffreestanding -O2 -nostdlib $(OBJ_LINK) -lgcc

# this is a pattern rule
$(OBJ)/%o:%c
	$(CC) $(CFLAGS) -c $< -o $@ -std=gnu99

./%.o:./%.s
	$(AS) $< -o $@

clean:
	rm -rf *.o
	rm $(TARGET)
