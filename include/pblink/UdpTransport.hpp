#ifndef PBLINK_UDP_TRANSPORT_HPP
#define PBLINK_UDP_TRANSPORT_HPP

#include <pblink/Transport.hpp>
#include <string>

namespace pblink {

class UdpTransport : public Transport {
public:
    UdpTransport(std::string host, int port);
    ~UdpTransport() override;

    bool open() override;
    void close() override;
    bool is_open() const override;

    ssize_t write(const uint8_t* buffer, size_t length) override;
    ssize_t read(uint8_t* buffer, size_t max_length, int timeout_ms) override;

private:
    std::string _host;
    int _port;
    int _sockfd{-1};
    bool _is_open{false};
};

} // namespace pblink

#endif // PBLINK_UDP_TRANSPORT_HPP
