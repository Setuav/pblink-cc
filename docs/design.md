# pblink-cc Design Specification

## Overview
`pblink-cc` is a lightweight, zero-dependency C++17 client library for Companion Computers (Raspberry Pi, Jetson, Orange Pi, NUC, etc.). It connects to PX4 Autopilot's `pblink` module over UDP or Serial (UART) and exposes PX4 uORB telemetry streams as native C++ types using Nanopb.

---

## PX4 Autopilot Module (`pblink`)

The companion library `pblink-cc` communicates directly with the **`pblink`** C++ module running inside PX4 Autopilot (`src/modules/pblink`).

### Key Characteristics of PX4 `pblink`:
- **Role:** High-speed, bidirectional uORB $\leftrightarrow$ Protobuf (Nanopb) bridge module for PX4.
- **Transports:** Supports POSIX UDP (for SITL & Ethernet) and NuttX/POSIX Serial UART (for hardware companion links).
- **Core Parameters:**
  - `PBLINK_EN`: Enable/disable the module (default: `1`).
  - `PBLINK_CFG`: Serial port or transport configuration mode (e.g. `1000` for UDP).
  - `PBLINK_PORT`: UDP port for companion link (default: `14556`).
  - `PBLINK_RMT_IP`: Companion Computer target IP address (default: `127.0.0.1` / `2130706433`).
- **Telemetry Auto-Start (POSIX / SITL):** Automatically initialized via `ROMFS/px4fmu_common/init.d-posix/rcS` when built with board variant `make px4_sitl_pblink`.

---

## Key Architectural Decisions

- **Language & Standard:** Modern C++17 (`-std=c++17`).
- **Target OS:** POSIX / Linux (Ubuntu, Raspberry Pi OS, Jetson L4T).
- **Serialization:** Nanopb (Zero-copy, header-only, no external `libprotobuf` dependency).
- **Transport:** POSIX Transport Abstraction (UDP & Serial UART).
- **Versioning Strategy:**
  - `main` branch tracks official PX4 `upstream/main`.
  - `release/1.18` branch for PX4 v1.18 release, etc.

---

## API Design (Hybrid Model: `on` + `read`)

### 1. Asynchronous Event Stream (`client.on<T>(callback)`)
For real-time event streaming, logging, and instant callback handling:
```cpp
client.on<pblink::msg::VehicleAttitude>([](const auto& att) {
    std::cout << "Attitude Quaternion W: " << att.q[0] << std::endl;
});
```

### 2. Synchronous Buffer Reading (`client.read(msg)`)
For control loops (e.g. 50Hz AI / trajectory planning loops) with zero thread-safety issues:
```cpp
pblink::msg::VehicleLocalPosition pos{};
if (client.read(pos)) {
    std::cout << "Current Z Position: " << pos.z << " m" << std::endl;
}
```

---

## Project Structure

```text
pblink-cc/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # Usage guide
├── design.md                   # System design specification
├── include/
│   └── pblink/
│       ├── client.hpp          # Client API interface
│       ├── transport.hpp       # Transport abstraction (UDP & Serial)
│       ├── messages.hpp        # C++ type aliases for Nanopb headers
│       └── proto/              # Generated Nanopb headers (matching PX4 main)
├── src/
│   ├── client.cpp              # Async RX loop & subscriber registry
│   └── transport.cpp           # UDP socket & POSIX termios serial implementation
└── examples/
    └── main.cpp                # Minimal usage example
```
