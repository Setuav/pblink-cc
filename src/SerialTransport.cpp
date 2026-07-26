#include <pblink/SerialTransport.hpp>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <iostream>

namespace pblink {

static speed_t get_termios_baudrate(int baudrate) {
    switch (baudrate) {
    case 9600:    return B9600;
    case 19200:   return B19200;
    case 38400:   return B38400;
    case 57600:   return B57600;
    case 115200:  return B115200;
    case 230400:  return B230400;
#ifdef B460800
    case 460800:  return B460800;
#endif
#ifdef B921600
    case 921600:  return B921600;
#endif
#ifdef B1500000
    case 1500000: return B1500000;
#endif
#ifdef B2000000
    case 2000000: return B2000000;
#endif
#ifdef B3000000
    case 3000000: return B3000000;
#endif
    default:
        std::cerr << "[pblink] Unsupported baudrate: " << baudrate << ", falling back to 115200" << std::endl;
        return B115200;
    }
}

SerialTransport::SerialTransport(std::string device, int baudrate)
    : _device(std::move(device)), _baudrate(baudrate), _fd(-1), _is_open(false) {}

SerialTransport::~SerialTransport() {
    close();
}

bool SerialTransport::open() {
    if (_is_open) return true;

    _fd = ::open(_device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (_fd < 0) {
        std::cerr << "[pblink] Failed to open serial device " << _device << ": " << std::strerror(errno) << std::endl;
        return false;
    }

    struct termios tty{};
    if (tcgetattr(_fd, &tty) != 0) {
        std::cerr << "[pblink] Failed to get serial attributes: " << std::strerror(errno) << std::endl;
        ::close(_fd);
        _fd = -1;
        return false;
    }

    speed_t speed = get_termios_baudrate(_baudrate);
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    // Set raw mode (no canonical processing, no echo, no signals)
    cfmakeraw(&tty);

    // 8N1
    tty.c_cflag &= ~PARENB;        // No parity
    tty.c_cflag &= ~CSTOPB;        // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;            // 8 bits
    tty.c_cflag &= ~CRTSCTS;       // No hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(_fd, TCSANOW, &tty) != 0) {
        std::cerr << "[pblink] Failed to set serial attributes: " << std::strerror(errno) << std::endl;
        ::close(_fd);
        _fd = -1;
        return false;
    }

    tcflush(_fd, TCIOFLUSH);
    _is_open = true;
    return true;
}

void SerialTransport::close() {
    if (_fd >= 0) {
        tcflush(_fd, TCIOFLUSH);
        ::close(_fd);
        _fd = -1;
    }
    _is_open = false;
}

bool SerialTransport::is_open() const {
    return _is_open;
}

ssize_t SerialTransport::write(const uint8_t* buffer, size_t length) {
    if (!_is_open || _fd < 0 || !buffer || length == 0) {
        return -1;
    }
    return ::write(_fd, buffer, length);
}

ssize_t SerialTransport::read(uint8_t* buffer, size_t max_length, int timeout_ms) {
    if (!_is_open || _fd < 0 || !buffer || max_length == 0) {
        return -1;
    }

    if (timeout_ms >= 0) {
        struct pollfd pfd{};
        pfd.fd = _fd;
        pfd.events = POLLIN;

        int ret = ::poll(&pfd, 1, timeout_ms);
        if (ret <= 0) {
            return ret; // 0 = timeout, -1 = error
        }
    }

    return ::read(_fd, buffer, max_length);
}

} // namespace pblink
