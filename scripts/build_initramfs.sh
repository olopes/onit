#!/bin/sh

export INITRAMFS_BUILD=`pwd`/build/initramfs
export BUSYBOX_BUILD=`pwd`/build/busybox
mkdir -p $INITRAMFS_BUILD
cd $INITRAMFS_BUILD
mkdir -p bin sbin etc proc sys usr/bin usr/sbin lib/modules dev var/tmp
cp -a $BUSYBOX_BUILD/_install/* .
cp ../onit .
ln -s onit init
KERN_NAME=`readlink /boot/vmlinuz | sed 's/vmlinuz-//'`
# cp -a "/lib/modules/$KERN_NAME" lib/modules/
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz

