#include <pblink/Client.hpp>
#include <iostream>

namespace pblink {

Client::Client(const std::string& url)
    : _url(url), _transport(create_transport_from_url(url)) {}

Client::~Client() {
    stop();
}

bool Client::start() {
    if (_is_running) {
        return true;
    }

    if (_transport && _transport->open()) {
        _is_running = true;
        std::cout << "[pblink::Client] Client started on " << _url << std::endl;
        return true;
    }

    return false;
}

void Client::stop() {
    if (!_is_running) {
        return;
    }

    _is_running = false;
    if (_transport) {
        _transport->close();
    }
    std::cout << "[pblink::Client] Client stopped." << std::endl;
}

bool Client::is_running() const {
    return _is_running;
}

} // namespace pblink
