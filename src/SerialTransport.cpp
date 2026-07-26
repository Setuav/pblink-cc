#include <pblink/SerialTransport.hpp>

namespace pblink {

SerialTransport::SerialTransport(std::string device, int baudrate)
    : _device(std::move(device)), _baudrate(baudrate) {}

SerialTransport::~SerialTransport() {
    close();
}

bool SerialTransport::open() {
    _is_open = true;
    return true;
}

void SerialTransport::close() {
    _is_open = false;
}

bool SerialTransport::is_open() const {
    return _is_open;
}

ssize_t SerialTransport::write(const uint8_t* buffer, size_t length) {
    (void)buffer;
    return static_cast<ssize_t>(length);
}

ssize_t SerialTransport::read(uint8_t* buffer, size_t max_length, int timeout_ms) {
    (void)buffer;
    (void)max_length;
    (void)timeout_ms;
    return 0;
}

} // namespace pblink
