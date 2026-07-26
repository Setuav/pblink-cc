#include <pblink/Decoder.hpp>
#include <iostream>
#include <cassert>
#include <cstring>

void test_frame_encoding_decoding() {
    pblink::Decoder decoder;

    bool callback_called = false;
    pblink::Frame decoded_frame{};

    decoder.set_callback([&](const pblink::Frame& frame) {
        callback_called = true;
        decoded_frame = frame;
    });

    uint8_t payload_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t frame_buffer[256];

    size_t encoded_len = pblink::Decoder::encode_frame(
        0x10, // msg_type_id
        42,   // sequence
        payload_data,
        sizeof(payload_data),
        frame_buffer,
        sizeof(frame_buffer)
    );

    assert(encoded_len == pblink::HEADER_SIZE + sizeof(payload_data) + pblink::CHECKSUM_SIZE);

    // Feed bytes to decoder
    decoder.process_bytes(frame_buffer, encoded_len);

    assert(callback_called);
    assert(decoded_frame.msg_type_id == 0x10);
    assert(decoded_frame.sequence == 42);
    assert(decoded_frame.payload_len == sizeof(payload_data));
    assert(decoded_frame.payload.size() == sizeof(payload_data));
    assert(std::memcmp(decoded_frame.payload.data(), payload_data, sizeof(payload_data)) == 0);

    std::cout << "[PASS] Frame encoding and decoding test successful!" << std::endl;
}

void test_corrupted_crc_rejection() {
    pblink::Decoder decoder;
    bool callback_called = false;

    decoder.set_callback([&](const pblink::Frame&) {
        callback_called = true;
    });

    uint8_t payload_data[] = {0xAA, 0xBB, 0xCC};
    uint8_t frame_buffer[256];

    size_t encoded_len = pblink::Decoder::encode_frame(
        0x20, 1, payload_data, sizeof(payload_data), frame_buffer, sizeof(frame_buffer)
    );

    // Corrupt the CRC byte
    frame_buffer[encoded_len - 1] ^= 0xFF;

    decoder.process_bytes(frame_buffer, encoded_len);

    assert(!callback_called);
    std::cout << "[PASS] Corrupted CRC rejection test successful!" << std::endl;
}

int main() {
    std::cout << "Running pblink Decoder tests..." << std::endl;
    test_frame_encoding_decoding();
    test_corrupted_crc_rejection();
    std::cout << "All Decoder tests passed!" << std::endl;
    return 0;
}
