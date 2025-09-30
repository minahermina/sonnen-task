#!/bin/sh

set -e

BUILDROOT_VERSION="2025.08"
BUILDROOT_TAR="https://buildroot.org/downloads/buildroot-$BUILDROOT_VERSION.tar.gz"
BUILDROOT_FILE="buildroot-$BUILDROOT_VERSION.tar.gz"
BUILDROOT_DIR="buildroot"


# 1- Downloading buildroot
if [ ! -f "$BUILDROOT_FILE" ]; then
    echo "====> Downloading $BUILDROOT_FILE"
    wget "$BUILDROOT_TAR" -O "$BUILDROOT_FILE"
else
    echo "====> $BUILDROOT_FILE already exists"
fi

# 2- Uncompressing buildroot source code to buildroot/ directory
if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "====> Uncompressing $BUILDROOT_FILE to $BUILDROOT_DIR/"
    mkdir "$BUILDROOT_DIR"
    tar xvf "$BUILDROOT_FILE" -C "$BUILDROOT_DIR" --strip-components=1
else
    echo "====> $BUILDROOT_DIR/ directory already exists"
fi

# 3 Copy buildroot config to buildroot/
cp .config "$BUILDROOT_DIR"
