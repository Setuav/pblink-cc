#include "pblink/Decoder.hpp"
#include <cstring>

namespace pblink {

uint16_t Decoder::calculate_crc16(const uint8_t* data, size_t length, uint16_t initial) {
    uint16_t crc = initial;
    const uint16_t poly = 0x1021u;

    for (size_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(static_cast<uint16_t>(data[i]) << 8u);
        for (int b = 0; b < 8; ++b) {
            if (crc & (1u << 15u)) {
                crc = static_cast<uint16_t>((crc << 1u) ^ poly);
            } else {
                crc = static_cast<uint16_t>(crc << 1u);
            }
        }
    }
    return crc;
}

uint8_t Decoder::calculate_header_crc(uint8_t msg_type, uint8_t seq, uint16_t payload_len) {
    uint8_t crc = 0;
    crc ^= FRAME_MAGIC_BYTE;
    crc ^= msg_type;
    crc ^= seq;
    crc ^= static_cast<uint8_t>(payload_len & 0xFF);
    crc ^= static_cast<uint8_t>((payload_len >> 8) & 0xFF);
    return crc;
}

size_t Decoder::encode_frame(uint8_t msg_type_id, uint8_t sequence,
                            const uint8_t* payload, uint16_t payload_len,
                            uint8_t* out_buffer, size_t max_out_len) {
    size_t total_size = HEADER_SIZE + payload_len + CHECKSUM_SIZE;
    if (max_out_len < total_size || out_buffer == nullptr) {
        return 0;
    }

    out_buffer[0] = FRAME_MAGIC_BYTE;
    out_buffer[1] = msg_type_id;
    out_buffer[2] = sequence;
    out_buffer[3] = static_cast<uint8_t>(payload_len & 0xFF);
    out_buffer[4] = static_cast<uint8_t>((payload_len >> 8) & 0xFF);
    out_buffer[5] = calculate_header_crc(msg_type_id, sequence, payload_len);

    if (payload_len > 0 && payload != nullptr) {
        std::memcpy(out_buffer + HEADER_SIZE, payload, payload_len);
    }

    uint16_t frame_crc = calculate_crc16(out_buffer + 1, HEADER_SIZE - 1 + payload_len);
    out_buffer[HEADER_SIZE + payload_len] = static_cast<uint8_t>(frame_crc & 0xFF);
    out_buffer[HEADER_SIZE + payload_len + 1] = static_cast<uint8_t>((frame_crc >> 8) & 0xFF);

    return total_size;
}

Decoder::Decoder(FrameCallback callback)
    : callback_(std::move(callback)) {
    reset();
}

void Decoder::set_callback(FrameCallback callback) {
    callback_ = std::move(callback);
}

void Decoder::reset() {
    state_ = State::SEARCH_MAGIC;
    rx_buffer_.clear();
    payload_buf_.clear();
    bytes_needed_ = 1;
}

void Decoder::process_bytes(const uint8_t* data, size_t length) {
    if (!data || length == 0) return;

    for (size_t i = 0; i < length; ++i) {
        uint8_t byte = data[i];

        switch (state_) {
        case State::SEARCH_MAGIC:
            if (byte == FRAME_MAGIC_BYTE) {
                rx_buffer_.clear();
                rx_buffer_.push_back(byte);
                state_ = State::READ_HEADER;
                bytes_needed_ = HEADER_SIZE - 1; // Remaining header bytes
            }
            break;

        case State::READ_HEADER:
            rx_buffer_.push_back(byte);
            if (rx_buffer_.size() == HEADER_SIZE) {
                msg_type_id_ = rx_buffer_[1];
                sequence_ = rx_buffer_[2];
                payload_len_ = static_cast<uint16_t>(rx_buffer_[3] | (rx_buffer_[4] << 8));
                uint8_t expected_header_crc = rx_buffer_[5];

                uint8_t calc_header_crc = calculate_header_crc(msg_type_id_, sequence_, payload_len_);
                if (calc_header_crc != expected_header_crc) {
                    reset();
                    break;
                }

                payload_buf_.clear();
                if (payload_len_ > 0) {
                    payload_buf_.reserve(payload_len_);
                    state_ = State::READ_PAYLOAD;
                    bytes_needed_ = payload_len_;
                } else {
                    state_ = State::READ_CHECKSUM;
                    bytes_needed_ = CHECKSUM_SIZE;
                }
            }
            break;

        case State::READ_PAYLOAD:
            rx_buffer_.push_back(byte);
            payload_buf_.push_back(byte);
            if (payload_buf_.size() == payload_len_) {
                state_ = State::READ_CHECKSUM;
                bytes_needed_ = CHECKSUM_SIZE;
            }
            break;

        case State::READ_CHECKSUM:
            rx_buffer_.push_back(byte);
            if (rx_buffer_.size() == HEADER_SIZE + payload_len_ + CHECKSUM_SIZE) {
                uint16_t received_crc = static_cast<uint16_t>(
                    rx_buffer_[HEADER_SIZE + payload_len_] |
                    (rx_buffer_[HEADER_SIZE + payload_len_ + 1] << 8)
                );

                uint16_t expected_crc = calculate_crc16(rx_buffer_.data() + 1, HEADER_SIZE - 1 + payload_len_);
                if (expected_crc == received_crc) {
                    if (callback_) {
                        Frame frame;
                        frame.msg_type_id = msg_type_id_;
                        frame.sequence = sequence_;
                        frame.payload_len = payload_len_;
                        frame.payload = payload_buf_;
                        callback_(frame);
                    }
                }
                reset();
            }
            break;
        }
    }
}

} // namespace pblink
