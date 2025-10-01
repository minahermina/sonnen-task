#!/bin/sh
set -e

ROOTFS_XZ="build/rootfs.ext2.xz"
ROOTFS="build/rootfs.ext2"
KERNEL="build/bzImage"

# Always refresh the rootfs: remove old one if exists, then decompress
if [ -f "$ROOTFS" ]; then
    echo "Removing old $ROOTFS..."
    rm -f "$ROOTFS"
fi

echo "Decompressing $ROOTFS_XZ..."
unxz "$ROOTFS_XZ" --keep

# Run QEMU with the built kernel and rootfs
echo "Starting QEMU..."
qemu-system-i386 \
  -kernel "$KERNEL" \
  -hda "$ROOTFS" \
  -append "root=/dev/sda rw console=ttyS0" \
  -m 512M \
  -nographic

