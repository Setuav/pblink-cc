#include <pblink/Client.hpp>
#include <iostream>
#include <cassert>

void test_client_lifecycle() {
    std::cout << "[Test] Running test_client_lifecycle..." << std::endl;

    pblink::Client client("udp://127.0.0.1:14556");
    assert(!client.is_running());

    bool ok = client.start();
    assert(ok);
    assert(client.is_running());

    client.stop();
    assert(!client.is_running());

    std::cout << "[Test] test_client_lifecycle PASSED." << std::endl;
}

int main() {
    std::cout << "=== Running pblink-cc Unit Tests ===" << std::endl;

    test_client_lifecycle();

    std::cout << "=== All pblink-cc Unit Tests Passed! ===" << std::endl;
    return 0;
}
