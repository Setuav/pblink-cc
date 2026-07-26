#include <pblink/Client.hpp>
#include <iostream>

int main() {
    std::cout << "[pblink-cc Example] Initializing client..." << std::endl;

    pblink::Client client("udp://127.0.0.1:14556");

    if (client.start()) {
        std::cout << "[pblink-cc Example] Client running successfully!" << std::endl;
    } else {
        std::cerr << "[pblink-cc Example] Failed to start client." << std::endl;
        return 1;
    }

    client.stop();
    return 0;
}
