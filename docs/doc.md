# Sonnen Energy Manager - Technical Documentation

This document provides technical information about the Sonnen Energy Manager system architecture, design decisions, and protocol specification.

## Table of Contents

- [Design Decisions & Justifications](#design-decisions--justifications)
  - [Programming Language: C](#programming-language-c)
  - [Build System: Buildroot](#build-system-buildroot)
  - [Communication Protocol: Unix Domain Sockets](#communication-protocol-unix-domain-sockets)
  - [Data Format: Custom Binary Format](#data-format-custom-binary-format)
- [System Architecture](#system-architecture)
  - [Overview](#overview)
  - [Component Responsibilities](#component-responsibilities)
  - [Communication Protocol](#communication-protocol)
  - [Communication Protocol Flow](#communication-protocol-flow)
  - [Error Handling](#error-handling)

## Design Decisions & Justifications

### Programming Language: C

**Justification**:

1. **Minimal Resource Footprint**: No runtime overhead, small binary size (~40KB combined).
2. **Familiarity**: This is the most programming language i am familiar with.

### Build System: Buildroot

**Justification**:

2. **Reproducibility**: Single `.config` file defines entire system.
5. **Simple Configuration**: Familiar Kconfig interface with single `.config` file.
6. **External Package System**: Clean integration via `BR2_EXTERNAL` mechanism

### Communication Protocol: Unix Domain Sockets

**Justification**: <br>
I searched about several protocols such as HTTP, gRPC, and MQTT, but found them too complex for the purpose of this simple task. Since both the server and client services run on the same machine, Unix Domain Sockets are the best option.

2. **Security**: Filesystem-based permissions (0700) using `umask`, no remote access, no network exposure.
3. **Reliability**: Connection-oriented (SOCK_STREAM), no packet loss, kernel-handled flow control
4. **Simplicity**: No IP addresses, ports, or firewall configuration needed
6. **Perfect for Local IPC**: Ideal for processes on same machine communicating frequently

### Data Format: Custom Binary Format

**Justification**:

I explored several data formats for passing data through Unix sockets. Initially, I considered using `JSON`, but it would introduce a large external dependency and add non-negligible serialization and deserialization overhead, which is unnecessary for a simple task like exchanging data between the client and server. I also looked into `Protobuf`, which offers better performance than JSON, but it adds complexity that is not justified when both the `client` and `server` run on the same machine.

For these reasons, I decided to implement a custom binary format using simple C structs.

1. **Efficiency**: Zero parsing overhead (direct struct copy), minimal size.
2. **Performance**: Direct memory mapping, no string parsing or dynamic allocation
3. **Determinism**: Fixed message sizes known at compile time, predictable memory usage
4. **No External Dependencies**: No JSON/XML libraries needed, zero runtime dependencies beyond libc

**Message Sizes**:
- Data Request: 9 bytes
- Data Response: 17 bytes  
- Battery Command: 14 bytes

**Conclusion**: For single application with same codebase, custom binary format is optimal for embedded targets.

## System Architecture

### Overview

The Sonnen Energy Manager is a client-server application that simulates a battery energy management system for solar installations.

```
┌────────────────────────────────────────────────────────────────────┐
│                         QEMU Environment                           │
│  ┌──────────────────────────────────────────────────────────────┐  │
│  │                      Linux System                            │  │
│  │  ┌─────────────────┐         ┌─────────────────┐             │  │
│  │  │                 │         │                 │             │  │
│  │  │  systemd unit   │         │  systemd unit   │             │  │
│  │  │ sonnen-server   │         │ sonnen-client   │             │  │
│  │  │                 │         │                 │             │  │
│  │  └─┬──────┬────────┘         └────────┬──────┬─┘             │  │
│  │    │      │                           │      │               │  │
│  │    │      │    Unix Domain Socket     │      │               │  │
│  │    │      │  /var/run/sonnen-         │      │               │  │
│  │    │      │    battery.sock           │      │               │  │
│  │    │      └───────────┬───────────────┘      │               │  │
│  │    │                  │                      │               │  │
│  │    │      ┌───────────▼────────────┐         │               │  │
│  │    │      │   Binary Protocol      │         │               │  │
│  │    │      │   - Data Request       │         │               │  │
│  │    │      │   - Data Response      │         │               │  │
│  │    │      │   - Battery Command    │         │               │  │
│  │    │      └────────────────────────┘         │               │  │
│  │    │                                         │               │  │
│  │  ┌──────────────────────────────────────────────────────────┐│  │
│  │  │                   syslog Logging                         ││  │
│  │  │                     journalctl                           ││  │
│  │  └──────────────────────────────────────────────────────────┘│  │
│  └──────────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────────┘
```

### Component Responsibilities

#### Server (`sonnen-server`)

**Purpose**: Simulates the solar installation hardware interface

**Responsibilities**:
- Listen for client connections on Unix domain socket
- Generate simulated PV production data (0-5000W)
- Generate simulated household consumption data (500-3000W)
- Respond to data requests
- Process battery commands
- Log all operations

**Lifecycle**:
1. Initialize socket at `/var/run/sonnen-battery.sock`
2. Enter main accept loop
3. Accept client connections
4. Process requests (data request or battery command)
5. Close client connection
6. Repeat until SIGTERM/SIGINT

#### Client (`sonnen-client`)

**Purpose**: Implements the energy management logic

**Responsibilities**:
- Connect to server socket periodically (every 5 seconds)
- Request current solar data (PV production, household consumption)
- Calculate optimal battery action based on energy balance
- Send battery commands to server (charge/discharge with power amount)

**Lifecycle**:
1. Connect to server
2. Request solar data
3. Calculate battery action
4. Send battery command
5. Sleep 5 seconds
6. Repeat until SIGTERM/SIGINT


### Communication Protocol

The client and server communicate via **Unix domain sockets** using a custom binary protocol. Each interaction follows a request-response pattern:

1. **Client** opens connection
2. **Client** sends request (data request or battery command)
3. **Server** processes request
4. **Server** sends response (for data requests)
5. Connection closes

The client initiates two separate connections per cycle:
- First connection: Request data
- Second connection: Send battery command

### Communication Protocol Flow

#### Complete Client Cycle

```
  ┌─────────────────────────────────────┐
  │                                     │
  │  1. Connect to server               │
  │  2. Send MsgDataRequest             │
  │  3. Receive MsgDataResponse         │
  │  4. Parse data                      │
  │  5. Calculate battery action        │
  │  6. Close connection                │
  │                                     │
  │  7. Connect to server (again)       │
  │  8. Send MsgBatteryCommand          │
  │  9. Close connection                │
  │                                     │
  │  10. Sleep 5 seconds                │
  │                                     │
  └──────────────┬──────────────────────┘
                 │
                 └──► Repeat
```

### Error Handling
- **Using syslog and use journalctl** <br> 
- **Signal Handling**:
  - `SIGINT` / `SIGTERM` → Graceful shutdown
