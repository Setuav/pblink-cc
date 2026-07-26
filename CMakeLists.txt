cmake_minimum_required(VERSION 3.14)
project(pblink-cc VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Core library sources
set(PBLINK_CC_SOURCES
    src/Client.cpp
    src/Transport.cpp
    src/UdpTransport.cpp
    src/SerialTransport.cpp
)

# Build static library
add_library(pblink_cc STATIC ${PBLINK_CC_SOURCES})

# Target include directories
target_include_directories(pblink_cc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Example executable
add_executable(pblink_example examples/Main.cpp)
target_link_libraries(pblink_example PRIVATE pblink_cc)

# Test executable
enable_testing()
add_executable(pblink_tests tests/TestClient.cpp)
target_link_libraries(pblink_tests PRIVATE pblink_cc)
add_test(NAME client_lifecycle_test COMMAND pblink_tests)
