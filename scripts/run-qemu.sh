#!/bin/sh

set -e

qemu-system-i386 \
  -kernel build/bzImage \
  -m 512M \
  -M pc \
  -hda "build/rootfs.ext2" \
  -append "root=/dev/sda rw console=ttyS0" \
  -nographic
