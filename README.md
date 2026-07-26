# pblink-cc

Zero-dependency C++17 Companion Computer SDK for PX4 `pblink` protocol.

## Features
- **High-speed Binary Bridge:** Direct uORB <-> Protobuf communication (50Hz–200Hz)
- **Zero External Dependencies:** Pure C++17 POSIX library, uses Nanopb for lightweight C struct serialization
- **Multi-Transport Support:** UDP (`udp://127.0.0.1:14556`) and Serial UART (`serial:///dev/ttyUSB0:115200`)
- **Hybrid API Model:** Asynchronous event callbacks (`client.on<T>()`) and lock-free buffer polling (`client.read(msg)`)

## Directory Structure
- `include/pblink/`: C++ header files (`Client.hpp`, `Transport.hpp`, `UdpTransport.hpp`, `SerialTransport.hpp`, `Messages.hpp`)
- `src/`: Implementation files (`Client.cpp`, `Transport.cpp`, `UdpTransport.cpp`, `SerialTransport.cpp`)
- `examples/`: Minimal usage examples (`Main.cpp`)
- `tests/`: Unit test suite (`TestClient.cpp`)
- `docs/`: Architecture specification (`architecture.md`) and design notes (`design.md`)

## Quick Start (Build & Test)

```bash
# Configure and build
cmake -B build
cmake --build build

# Run example application
./build/pblink_example

# Run unit tests
./build/pblink_tests
```
