#ifndef PBLINK_SERIAL_TRANSPORT_HPP
#define PBLINK_SERIAL_TRANSPORT_HPP

#include <pblink/Transport.hpp>
#include <string>

namespace pblink {

class SerialTransport : public Transport {
public:
    SerialTransport(std::string device, int baudrate);
    ~SerialTransport() override;

    bool open() override;
    void close() override;
    bool is_open() const override;

    ssize_t write(const uint8_t* buffer, size_t length) override;
    ssize_t read(uint8_t* buffer, size_t max_length, int timeout_ms) override;

private:
    std::string _device;
    int _baudrate;
    int _fd{-1};
    bool _is_open{false};
};

} // namespace pblink

#endif // PBLINK_SERIAL_TRANSPORT_HPP
