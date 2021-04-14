#! /bin/bash

sbin='kernel/mos.kernel'
osiso='mos.iso'
mkdir -p isodir/boot/grub
cp $osbin isodir/boot/$osbin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o $osiso isodir

