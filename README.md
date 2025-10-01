# Sonnen Energy Manager

## Overview

This project simulates a solar energy management system where:
- A **server** generates simulated PV production and household consumption data
- A **client** analyzes the data and calculates optimal battery charging/discharging actions
- Communication happens via **Unix domain sockets** using a custom binary data format
- Both services run as systemd units in a minimal Linux environment built with **Buildroot**

## Table of Contents

- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Building the Project](#building-the-project)
- [Running the Project](#running-the-project)
- [Usage](#usage)
- [Documentation](#documentation)
- [Troubleshooting](#troubleshooting)

### Message Flow

1. **Data Request**: Client → Server
   - Client requests current PV and consumption data
   
2. **Data Response**: Server → Client
   - Server responds with simulated solar data
   
3. **Battery Command**: Client → Server
   - Client calculates and sends charge/discharge command

## Project Structure

```
.
├── build/                      # Build output (kernel, rootfs, etc.)
│   ├── bzImage                 # Linux kernel image
│   └── rootfs.ext2.xz          # Compressed rootfs
│
├── docs/                       # Project documentation
│
├── scripts/
│   ├── run-qemu.sh             # Starts QEMU with kernel + rootfs
│   └── setup-buildroot.sh      # Downloads and configures Buildroot
│
└── sonnen-energy-manager/      # Buildroot external package
    ├── Config.in
    ├── external.desc
    ├── external.mk
    └── package/
        └── sonnen-energy-manager/
            ├── Config.in
            ├── sonnen-energy-manager.mk
            └── src/
                ├── Makefile
                ├── client.c            # Client implementation
                ├── server.c            # Server implementation
                ├── protocol.c          # Protocol implementation
                ├── protocol.h          # Protocol definitions
                ├── utils.c             # Utility functions
                ├── utils.h             # Utility headers
                ├── sonnen.h            # Main header
                ├── sonnen-client.service  # Systemd unit (client)
                └── sonnen-server.service  # Systemd unit (server)
```

## Prerequisites

To build and run this project, you need:

- **QEMU** (i386 system emulation)
- **xz-utils** (for decompressing rootfs)

### Installation

**Debian/Ubuntu (apt)**:
```bash
sudo apt install qemu-system-x86 xz-utils
```

**Void Linux (xbps)**:
```bash
sudo xbps-install qemu-system-i386 xz
```

**Fedora/RHEL (dnf)**:
```bash
sudo dnf install qemu-system-x86 xz
```

**Arch Linux (pacman)**:
```bash
sudo pacman -S qemu-system-x86 xz
```

## Running the Project

Start the emulated system:

```bash
./scripts/run-qemu.sh
```

This will:
1. Decompress `build/rootfs.ext2.xz` to `build/rootfs.ext2`
2. Start QEMU with the Linux kernel and rootfs
3. Boot into a minimal Linux system with serial console

### System Login

- **Username**: `root`
- **Password**: (none - press Enter)

## Usage

### Checking Service Status

Once logged in, check if services are running:

```bash
systemctl status sonnen-server
systemctl status sonnen-client
```

### Viewing Logs

**Real-time logs**:
```bash
journalctl -f -u sonnen-server -u sonnen-client
```

### Example Output

```
sonnen-server: [SERVER] Data Request - PV: 3421W, Consumption: 1876W
sonnen-client: [CLIENT] PV: 3421W, Consumption: 1876W -> charge 1545W
sonnen-server: [SERVER] Battery Command: charge 1545W

sonnen-server: [SERVER] Data Request - PV: 1234W, Consumption: 2567W
sonnen-client: [CLIENT] PV: 1234W, Consumption: 2567W -> discharge 1333W
sonnen-server: [SERVER] Battery Command: discharge 1333W
```

**Server logs only**:
```bash
journalctl -u sonnen-server
```

**Client logs only**:
```bash
journalctl -u sonnen-client
```

## Documentation

For detailed technical documentation, including protocol specification, development guidelines, and architecture details, see:

📖 **[docs/doc.md](docs/doc.md)**

## Acknowledgments

Built with:
- **Buildroot** - Embedded Linux build system
- **QEMU** - Machine emulator and virtualizer
- **systemd** - System and service manager
