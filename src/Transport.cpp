#include <pblink/Transport.hpp>
#include <pblink/UdpTransport.hpp>
#include <pblink/SerialTransport.hpp>

namespace pblink {

std::unique_ptr<Transport> create_transport_from_url(const std::string& url) {
    if (url.rfind("udp://", 0) == 0) {
        return std::make_unique<UdpTransport>("127.0.0.1", 14556);
    } else if (url.rfind("serial://", 0) == 0) {
        return std::make_unique<SerialTransport>("/dev/ttyUSB0", 115200);
    }
    return std::make_unique<UdpTransport>("127.0.0.1", 14556);
}

} // namespace pblink
