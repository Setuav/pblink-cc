#pragma once

#include <atomic>
#include <cstdint>
#include <cstddef>
#include <chrono>

namespace pblink {

inline uint64_t get_monotonic_time_us() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

template <typename T>
class TopicBuffer {
public:
    TopicBuffer() = default;

    void write(const T& msg) {
        size_t next_idx = 1 - active_read_idx_.load(std::memory_order_relaxed);
        buffer_[next_idx] = msg;
        timestamp_us_[next_idx] = get_monotonic_time_us();
        active_read_idx_.store(next_idx, std::memory_order_release);
        has_data_.store(true, std::memory_order_release);
    }

    bool read(T& out_msg, uint64_t* out_timestamp_us = nullptr) const {
        if (!has_data_.load(std::memory_order_acquire)) {
            return false;
        }
        size_t idx = active_read_idx_.load(std::memory_order_acquire);
        out_msg = buffer_[idx];
        if (out_timestamp_us) {
            *out_timestamp_us = timestamp_us_[idx];
        }
        return true;
    }

    bool has_data() const {
        return has_data_.load(std::memory_order_acquire);
    }

    void reset() {
        has_data_.store(false, std::memory_order_release);
        active_read_idx_.store(0, std::memory_order_release);
    }

private:
    T buffer_[2]{};
    uint64_t timestamp_us_[2]{0, 0};
    std::atomic<size_t> active_read_idx_{0};
    std::atomic<bool> has_data_{false};
};

} // namespace pblink
