#!/bin/sh

export BUSYBOX_BUILD=`pwd`/build/busybox
mkdir -p $BUSYBOX_BUILD
mkdir busybox
pushd busybox/
wget https://busybox.net/downloads/busybox-1.29.3.tar.bz2
wget https://busybox.net/downloads/busybox-1.29.3.tar.bz2.sha256
sha256sum -c busybox-1.29.3.tar.bz2.sha256 
tar -xf busybox-1.29.3.tar.bz2 
cd busybox-1.29.3/
make O=$BUSYBOX_BUILD defconfig
cd $BUSYBOX_BUILD
# make menuconfig
sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config
make -j8
make install
popd
