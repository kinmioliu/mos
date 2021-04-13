#! /bin/bash
#container=$1
container='suspicious_archimedes'
for file in `ls`;
do
    docker cp $file $container:/home/os/
done
docker cp kernel $container:/home/os/
docker cp libc $container:/home/os/

for src_file in `ls *.c`;
do
    docker cp $src_file $container:/home/os/
done
for ld_file in `ls *.ld`;
do
    docker cp $ld_file $container:/home/os/
done
for as_file in `ls *.s`;
do
    docker cp $as_file $container:/home/os/
done
