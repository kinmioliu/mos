#! /bin/bash

cd kernel
make all
cd ..
sbin='kernel/mos.kernel'
osiso='mos.iso'
rm -rf isodir
mkdir -p isodir/boot/grub
cp kernel/mos.kernel isodir/boot/mos.kernel
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o $osiso isodir

