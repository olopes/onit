#!/bin/sh
# qemu-system-x86_64 -kernel /boot/vmlinuz -initrd initramfs.cpio.gz -nographic -append "console=ttyS0"
qemu-system-x86_64 -kernel /boot/vmlinuz -initrd build/initramfs.cpio.gz -nographic -append "console=ttyS0 init=/onit root=/dev/sda1" -enable-kvm
