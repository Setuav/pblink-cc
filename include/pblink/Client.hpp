#ifndef PBLINK_CLIENT_HPP
#define PBLINK_CLIENT_HPP

#include <string>
#include <memory>
#include <functional>
#include <atomic>
#include <pblink/Transport.hpp>
#include <pblink/Messages.hpp>

namespace pblink {

class Client {
public:
    explicit Client(const std::string& url = "udp://127.0.0.1:14556");
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool start();
    void stop();
    bool is_running() const;

    template<typename T>
    bool read(T& msg_out) {
        (void)msg_out;
        return false;
    }

    template<typename T>
    void on(std::function<void(const T&)> callback) {
        (void)callback;
    }

private:
    std::string _url;
    std::unique_ptr<Transport> _transport;
    std::atomic<bool> _is_running{false};
};

} // namespace pblink

#endif // PBLINK_CLIENT_HPP
