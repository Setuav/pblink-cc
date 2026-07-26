#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <functional>

namespace pblink {

constexpr uint8_t FRAME_MAGIC_BYTE = 0xAA;
constexpr size_t HEADER_SIZE = 6;
constexpr size_t CHECKSUM_SIZE = 2;

struct Frame {
    uint8_t msg_type_id{0};
    uint8_t sequence{0};
    uint16_t payload_len{0};
    std::vector<uint8_t> payload;
};

using FrameCallback = std::function<void(const Frame& frame)>;

class Decoder {
public:
    enum class State {
        SEARCH_MAGIC,
        READ_HEADER,
        READ_PAYLOAD,
        READ_CHECKSUM
    };

    explicit Decoder(FrameCallback callback = nullptr);

    void set_callback(FrameCallback callback);
    void process_bytes(const uint8_t* data, size_t length);
    void reset();

    // Helper functions for CRC calculation
    static uint16_t calculate_crc16(const uint8_t* data, size_t length, uint16_t initial = 0xFFFF);
    static uint8_t calculate_header_crc(uint8_t msg_type, uint8_t seq, uint16_t payload_len);

    // Frame encoder
    static size_t encode_frame(uint8_t msg_type_id, uint8_t sequence,
                               const uint8_t* payload, uint16_t payload_len,
                               uint8_t* out_buffer, size_t max_out_len);

private:
    State state_{State::SEARCH_MAGIC};
    FrameCallback callback_{nullptr};

    std::vector<uint8_t> rx_buffer_;
    uint8_t header_buf_[HEADER_SIZE];
    size_t bytes_needed_{0};

    uint8_t msg_type_id_{0};
    uint8_t sequence_{0};
    uint16_t payload_len_{0};
    std::vector<uint8_t> payload_buf_;
    uint8_t checksum_buf_[CHECKSUM_SIZE];
};

} // namespace pblink
