# pblink & pblink-cc Architecture Specification

## 1. Executive Summary & Ecosystem Overview

The `pblink` ecosystem provides a unified, high-speed, zero-dependency binary bridge between **PX4 Autopilot** and **Companion Computers** (Raspberry Pi, Jetson Nano/Orin, NUC, Orange Pi, etc.). It replaces heavy ROS 2 / Micro-XRCE-DDS stacks for direct C++ embedded control (50Hz–200Hz).

The ecosystem is composed of **three decoupled repositories**:

```text
                     +----------------------------------+
                     |           pblink-proto           |
                     |  Single Source of Truth Schemas  |
                     |  (topics.yaml, .proto, .options) |
                     +----------------------------------+
                               /              \
               Git Submodule  /                \ Referenced
                             v                  v
+------------------------------------+  +------------------------------------+
|             Autopilot              |  |             pblink-cc              |
|        (src/modules/pblink)        |  |        Companion C++17 SDK         |
|   uORB <-> Nanopb PX4 C++ Module   |  |   Zero-Dependency Linux Client     |
+------------------------------------+  +------------------------------------+
```

1. **`pblink-proto`** (`Setuav/pblink-proto`): Shared protocol repository holding `topics.yaml`, `.proto` definitions, and Nanopb `.options` files.
2. **`Autopilot`** (`Setuav/Autopilot` -> `src/modules/pblink`): PX4 C++ module executing inside NuttX/POSIX that streams uORB topics over UDP/UART.
3. **`pblink-cc`** (`Setuav/pblink-cc`): Zero-dependency C++17 client library for Companion Computers providing event-driven callbacks and lock-free buffer polling.

---

## 2. Binary Framing & Serialization Protocol

### 2.1 Frame Layout
Every packet transmitted over UDP or UART adheres to the following byte layout:

```text
+---------+-------------+---------+---------------+------------+---------------+---------------+
|  MAGIC  | MSG_TYPE_ID |   SEQ   |  PAYLOAD_LEN  | HEADER_CRC |    PAYLOAD    |  PAYLOAD_CRC  |
| (1 Byte)|  (1 Byte)   | (1 Byte)|   (2 Bytes)   |  (1 Byte)  |   (N Bytes)   |   (2 Bytes)   |
+---------+-------------+---------+---------------+------------+---------------+---------------+
  0xFE       Topic ID    0..255    Little-Endian     XOR 0..4     Nanopb Bytes   CRC16-CCITT
```

- **MAGIC (`0xFE`):** Frame boundary identifier for stream synchronization.
- **HEADER_CRC:** Bitwise XOR checksum of bytes `0..4` for instant header validation.
- **PAYLOAD_CRC:** 16-bit CCITT CRC checksum over the raw payload buffer.

### 2.2 Topic ID Allocation Policy
- **Uplink Telemetry (PX4 $\rightarrow$ Companion):** `0x01` to `0x7F` (e.g. `0x01` Local Position, `0x03` Attitude, `0x09` Sensor Combined).
- **Downlink & Control (Companion $\rightarrow$ PX4):** `0x80` to `0xFF` (e.g. `0x81` Trajectory Setpoint, `0x8B` SubscriptionRequest, `0x90` LinkStatusRequest).

---

## 3. Administrative Protocol RPCs & System Messages

### 3.1 Dynamic Rate Adjustment (`SubscriptionRequest` - `0x8B`)
Companion can adjust the streaming frequency of any uORB topic live without restarting PX4:
- `msg_type_id`: Target topic ID (`0x03` for Attitude).
- `interval_us`: Period in microseconds (`20000 us` = 50Hz, `0 us` = Disable topic).

### 3.2 High-Precision Clock Synchronization (`TimeSync` - `0x8E`)
Computes microsecond time offset and clock drift between Linux system clock and PX4 `hrt_absolute_time()`:
- Companion sends `tc1` (Companion timestamp).
- PX4 echoes `tc1` and appends `ts1` (PX4 `hrt_absolute_time()`).

### 3.3 Remote Parameter RPCs (`0x8C`, `0x8D`, `0x15`)
Provides remote read/write access to PX4 parameters over Protobuf (`ParameterRequest`, `ParameterSet`, `ParameterValue`).
- Parameter names are constrained to `max_size: 16` (`char param_name[17]`).

### 3.4 Dynamic Topic Discovery RPCs (`0x8F`, `0x16`)
- `TopicListRequest` (`0x8F`): Companion queries available topics on demand.
- `TopicListResponse` (`0x16`): PX4 returns a single batch Protobuf array containing all topic IDs, string names, and current streaming Hz rates.

### 3.5 Link Performance Telemetry RPCs (`0x90`, `0x17`)
- `LinkStatusRequest` (`0x90`): Companion queries link health.
- `LinkStatusResponse` (`0x17`): PX4 returns real-time TX/RX kB/s throughput, message counts, drop counts, CRC error counts, and active topic counts.

---

## 4. Companion C++ SDK Architecture (`pblink-cc`)

`pblink-cc` is organized into **four distinct layers**:

```text
+-----------------------------------------------------------------------+
|                       User Companion Application                      |
|       (YOLO, OpenCV, Path Planner, Control Loop, Logger, ROS 2)        |
+-----------------------------------------------------------------------+
                                   │
                                   ▼
+-----------------------------------------------------------------------+
|                    Layer 1: Client API (pblink::Client)               |
|   client.on<VehicleAttitude>(fn)   │   if (client.read(local_pos))    |
+-----------------------------------------------------------------------+
                                   │
                                   ▼
+-----------------------------------------------------------------------+
|             Layer 2: Internal Dispatcher & Lock-Free Cache            |
|   - Lock-Free Double-Buffered Topic Cache (for fast read(msg))        |
|   - Callback Registry & Subscriber Dispatcher (for on<T>(fn))         |
|   - Background RX Thread (std::thread + POSIX poll loop)              |
|   - Nanopb Decoder (Zero-copy parse from raw UDP/Serial frames)       |
+-----------------------------------------------------------------------+
                                   │
                                   ▼
+-----------------------------------------------------------------------+
|               Layer 3: Transport Layer (pblink::Transport)            |
|                Transport (Base Abstract C++ Class)                    |
|        ┌───────────────────────────────────┴───────────────────┐      |
|        ▼                                                       ▼      |
|  UDPTransport (POSIX UDP)                   SerialTransport (UART)    |
|  "udp://127.0.0.1:14556"                    "serial:///dev/ttyTHS1"   |
+-----------------------------------------------------------------------+
                                   │
                                   ▼
+-----------------------------------------------------------------------+
|                      Layer 4: Physical Hardware                       |
|               UDP Socket / Ethernet / UART Serial Cable               |
+-----------------------------------------------------------------------+
```

### 4.1 Hybrid API Model (`on` + `read`)
`pblink-cc` offers a dual API pattern to satisfy both real-time event logging and 50Hz control loops:

1. **Asynchronous Event Stream (`client.on<T>(callback)`):**
   ```cpp
   client.on<pblink::msg::VehicleAttitude>([](const auto& att) {
       std::cout << "Attitude Q[0]: " << att.q[0] << std::endl;
   });
   ```
2. **Synchronous Lock-Free Polling (`client.read(msg)`):**
   ```cpp
   pblink::msg::VehicleLocalPosition pos{};
   if (client.read(pos)) {
       std::cout << "Current Z Position: " << pos.z << " m" << std::endl;
   }
   ```

---

## 5. Versioning & Release Branch Strategy

To ensure absolute compatibility between PX4 releases and Companion Computer applications:

- **`main` branch:** Tracks official PX4 `upstream/main` development branch.
- **`release/1.18` branch:** Fixed release branch created when PX4 v1.18 is officially tagged, preserving frozen uORB message schemas.
