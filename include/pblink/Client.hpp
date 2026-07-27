#pragma once

#include <string>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <pblink/Transport.hpp>
#include <pblink/Messages.hpp>
#include <pblink/TopicTraits.hpp>
#include <pblink/TopicBuffer.hpp>
#include <pblink/Decoder.hpp>

namespace pblink {

class Client {
public:
    explicit Client(const std::string& url = "udp://127.0.0.1:14556");
    explicit Client(std::unique_ptr<Transport> transport);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool start();
    void stop();
    bool is_running() const;

    // Polling API: Non-blocking, lock-free latest state read
    template <typename T>
    bool get_latest(T& out_msg, uint64_t* out_timestamp_us = nullptr) const {
        uint8_t id = TopicTraits<T>::msg_type_id;
        const auto* buf = get_buffer<T>(id);
        if (!buf) return false;
        return buf->read(out_msg, out_timestamp_us);
    }

    // Callback API: Register a callback for incoming messages
    template <typename T>
    void subscribe(std::function<void(const T&)> callback) {
        uint8_t id = TopicTraits<T>::msg_type_id;
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        callbacks_[id].push_back([callback](const uint8_t* payload, uint16_t len) {
            T msg = TopicTraits<T>::init_default();
            pb_istream_t stream = pb_istream_from_buffer(payload, len);
            if (pb_decode(&stream, TopicTraits<T>::fields(), &msg)) {
                callback(msg);
            }
        });
    }

    // Transmit API: Encode & send Nanopb message to PX4
    template <typename T>
    bool send(const T& msg) {
        uint8_t id = TopicTraits<T>::msg_type_id;
        return send_msg(id, TopicTraits<T>::fields(), &msg);
    }

    // High-level RPC Helpers
    bool request_topic_list();
    bool request_link_status();
    bool send_offboard_control_mode(bool position, bool velocity, bool acceleration, bool attitude, bool body_rate);
    bool send_vehicle_command(uint32_t command, float param1 = 0.0f, float param2 = 0.0f,
                               float param3 = 0.0f, float param4 = 0.0f,
                               double param5 = 0.0, double param6 = 0.0, float param7 = 0.0f);

private:
    void rx_thread_func();
    void handle_decoded_frame(const Frame& frame);
    bool send_msg(uint8_t msg_type_id, const pb_msgdesc_t* fields, const void* src_struct);

    template <typename T>
    const TopicBuffer<T>* get_buffer(uint8_t id) const {
        auto it = buffers_.find(id);
        if (it == buffers_.end() || !it->second) return nullptr;
        return static_cast<const TopicBuffer<T>*>(it->second.get());
    }

    template <typename T>
    TopicBuffer<T>* get_or_create_buffer(uint8_t id) {
        auto it = buffers_.find(id);
        if (it != buffers_.end()) {
            return static_cast<TopicBuffer<T>*>(it->second.get());
        }
        auto new_buf = std::make_unique<TopicBuffer<T>>();
        auto* ptr = new_buf.get();
        buffers_[id] = std::move(new_buf);
        return ptr;
    }

    void init_buffers();

    std::string url_;
    std::unique_ptr<Transport> transport_;
    Decoder decoder_;
    uint8_t tx_sequence_{0};

    std::atomic<bool> is_running_{false};
    std::thread rx_thread_;

    using RawCallback = std::function<void(const uint8_t* payload, uint16_t len)>;
    mutable std::mutex callbacks_mutex_;
    std::unordered_map<uint8_t, std::vector<RawCallback>> callbacks_;

    std::unordered_map<uint8_t, std::shared_ptr<void>> buffers_;
};

} // namespace pblink
