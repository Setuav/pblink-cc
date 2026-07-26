cmake_minimum_required(VERSION 3.14)
project(pblink-cc VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Directories
set(NANOPB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/nanopb)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${GENERATED_DIR})

# Gather all .proto files from proto/custom and proto/generated
file(GLOB CUSTOM_PROTO_FILES "${PROTO_DIR}/custom/*.proto")
file(GLOB GENERATED_PROTO_FILES "${PROTO_DIR}/generated/*.proto")
set(ALL_PROTO_FILES ${CUSTOM_PROTO_FILES} ${GENERATED_PROTO_FILES})

set(NANOPB_GENERATED_SRCS "")
set(NANOPB_GENERATED_HDRS "")

foreach(PROTO_FILE ${ALL_PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(PB_C "${GENERATED_DIR}/${PROTO_NAME}.pb.c")
    set(PB_H "${GENERATED_DIR}/${PROTO_NAME}.pb.h")

    add_custom_command(
        OUTPUT ${PB_C} ${PB_H}
        COMMAND ${Python3_EXECUTABLE} ${NANOPB_DIR}/generator/nanopb_generator.py
            --output-dir=${GENERATED_DIR}
            -I${PROTO_DIR}/custom
            -I${PROTO_DIR}/generated
            -I${NANOPB_DIR}/generator
            ${PROTO_FILE}
        DEPENDS ${PROTO_FILE} ${NANOPB_DIR}/generator/nanopb_generator.py
        COMMENT "Generating Nanopb bindings for ${PROTO_NAME}.proto"
    )
    list(APPEND NANOPB_GENERATED_SRCS ${PB_C})
    list(APPEND NANOPB_GENERATED_HDRS ${PB_H})
endforeach()

# Nanopb core sources
set(NANOPB_CORE_SOURCES
    ${NANOPB_DIR}/pb_encode.c
    ${NANOPB_DIR}/pb_decode.c
    ${NANOPB_DIR}/pb_common.c
)

# Core library sources
set(PBLINK_CC_SOURCES
    src/Client.cpp
    src/Transport.cpp
    src/UdpTransport.cpp
    src/SerialTransport.cpp
    src/Decoder.cpp
    ${NANOPB_CORE_SOURCES}
    ${NANOPB_GENERATED_SRCS}
)

# Build static library
add_library(pblink_cc STATIC ${PBLINK_CC_SOURCES})

# Target include directories
target_include_directories(pblink_cc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${GENERATED_DIR}>
    $<BUILD_INTERFACE:${NANOPB_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Example executable
add_executable(pblink_example examples/Main.cpp)
target_link_libraries(pblink_example PRIVATE pblink_cc)

# Test executables
enable_testing()
add_executable(pblink_tests tests/TestClient.cpp)
target_link_libraries(pblink_tests PRIVATE pblink_cc)
add_test(NAME client_lifecycle_test COMMAND pblink_tests)

add_executable(pblink_decoder_tests tests/TestDecoder.cpp)
target_link_libraries(pblink_decoder_tests PRIVATE pblink_cc)
add_test(NAME decoder_test COMMAND pblink_decoder_tests)
