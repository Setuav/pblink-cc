#ifndef PBLINK_TRANSPORT_HPP
#define PBLINK_TRANSPORT_HPP

#include <string>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace pblink {

enum class TransportType {
    UDP,
    SERIAL,
    UNKNOWN
};

class Transport {
public:
    virtual ~Transport() = default;

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool is_open() const = 0;

    virtual ssize_t write(const uint8_t* buffer, size_t length) = 0;
    virtual ssize_t read(uint8_t* buffer, size_t max_length, int timeout_ms) = 0;
};

std::unique_ptr<Transport> create_transport_from_url(const std::string& url);

} // namespace pblink

#endif // PBLINK_TRANSPORT_HPP
