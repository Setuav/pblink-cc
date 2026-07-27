#include <pblink/UdpTransport.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <iostream>

namespace pblink {

UdpTransport::UdpTransport(std::string host, int port)
    : _host(std::move(host)), _port(port), _sockfd(-1), _is_open(false) {}

UdpTransport::~UdpTransport() {
    close();
}

bool UdpTransport::open() {
    if (_is_open) return true;

    _sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd < 0) {
        std::cerr << "[pblink] Failed to create UDP socket: " << std::strerror(errno) << std::endl;
        return false;
    }

    int reuse = 1;
    ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#ifdef SO_REUSEPORT
    ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
#endif

    // Bind to _port + 1 (14557)
    struct sockaddr_in local_addr{};
    std::memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(static_cast<uint16_t>(_port + 1));

    if (::bind(_sockfd, reinterpret_cast<struct sockaddr*>(&local_addr), sizeof(local_addr)) < 0) {
        std::cerr << "[pblink] Failed to bind UDP port " << (_port + 1) << ": " << std::strerror(errno) << ", falling back" << std::endl;
        local_addr.sin_port = 0;
        ::bind(_sockfd, reinterpret_cast<struct sockaddr*>(&local_addr), sizeof(local_addr));
    } else {
        std::cout << "[pblink] UDP transport listening on port " << (_port + 1) << ", target " << _host << ":" << _port << std::endl;
    }

    // Set non-blocking mode
    int flags = ::fcntl(_sockfd, F_GETFL, 0);
    if (flags >= 0) {
        ::fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);
    }

    _is_open = true;
    return true;
}

void UdpTransport::close() {
    if (_sockfd >= 0) {
        ::close(_sockfd);
        _sockfd = -1;
    }
    _is_open = false;
}

bool UdpTransport::is_open() const {
    return _is_open;
}

ssize_t UdpTransport::write(const uint8_t* buffer, size_t length) {
    if (!_is_open || _sockfd < 0 || !buffer || length == 0) {
        return -1;
    }

    struct sockaddr_in dest_addr{};
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(static_cast<uint16_t>(_port));

    if (inet_pton(AF_INET, _host.c_str(), &dest_addr.sin_addr) <= 0) {
        std::cerr << "[pblink] Invalid target IP address: " << _host << std::endl;
        return -1;
    }

    ssize_t sent = ::sendto(_sockfd, buffer, length, 0,
                           reinterpret_cast<struct sockaddr*>(&dest_addr),
                           sizeof(dest_addr));
    return sent;
}

ssize_t UdpTransport::read(uint8_t* buffer, size_t max_length, int timeout_ms) {
    if (!_is_open || _sockfd < 0 || !buffer || max_length == 0) {
        return -1;
    }

    if (timeout_ms >= 0) {
        struct pollfd pfd{};
        pfd.fd = _sockfd;
        pfd.events = POLLIN;

        int ret = ::poll(&pfd, 1, timeout_ms);
        if (ret <= 0) {
            return ret; // 0 = timeout, -1 = error
        }
    }

    struct sockaddr_in src_addr{};
    socklen_t addr_len = sizeof(src_addr);
    ssize_t received = ::recvfrom(_sockfd, buffer, max_length, 0,
                                  reinterpret_cast<struct sockaddr*>(&src_addr),
                                  &addr_len);
    return received;
}

} // namespace pblink
