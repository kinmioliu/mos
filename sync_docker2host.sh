#! /bin/bash
container='suspicious_archimedes'
docker cp $container:/home/os/kernel/kernel/kernel.c ./kernel/kernel/kernel.c
docker cp $container:/home/os/kernel/kernel/idts.h ./kernel/kernel/idts.h
docker cp $container:/home/os/kernel/kernel/idts.c ./kernel/kernel/idts.c
docker cp $container:/home/os/kernel/kernel/mm.h ./kernel/kernel/mm.h
docker cp $container:/home/os/kernel/kernel/mm.c ./kernel/kernel/mm.c
docker cp $container:/home/os/kernel/kernel/pic.c ./kernel/kernel/pic.c
docker cp $container:/home/os/kernel/arch/i386/gdt.s ./kernel/arch/i386/gdt.s
docker cp $container:/home/os/kernel/arch/i386/boot.s ./kernel/arch/i386/boot.s
docker cp $container:/home/os/kernel/arch/i386/idt.s ./kernel/arch/i386/idt.s

