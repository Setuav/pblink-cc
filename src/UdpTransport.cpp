#include <pblink/UdpTransport.hpp>

namespace pblink {

UdpTransport::UdpTransport(std::string host, int port)
    : _host(std::move(host)), _port(port) {}

UdpTransport::~UdpTransport() {
    close();
}

bool UdpTransport::open() {
    _is_open = true;
    return true;
}

void UdpTransport::close() {
    _is_open = false;
}

bool UdpTransport::is_open() const {
    return _is_open;
}

ssize_t UdpTransport::write(const uint8_t* buffer, size_t length) {
    (void)buffer;
    return static_cast<ssize_t>(length);
}

ssize_t UdpTransport::read(uint8_t* buffer, size_t max_length, int timeout_ms) {
    (void)buffer;
    (void)max_length;
    (void)timeout_ms;
    return 0;
}

} // namespace pblink
