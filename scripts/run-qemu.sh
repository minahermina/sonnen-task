#!/bin/sh

set -e

qemu-system-i386 \
  -kernel build/bzImage \
  -hda build/rootfs.ext2 \
  -append "root=/dev/sda rw console=ttyS0" \
  -m 512M \
  -nographic \
