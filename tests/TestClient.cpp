#include <pblink/Client.hpp>
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

void test_client_lifecycle() {
    pblink::Client client("udp://127.0.0.1:14556");

    assert(!client.is_running());
    bool ok = client.start();
    assert(ok);
    assert(client.is_running());

    pblink::VehicleAttitude att{};
    // Initially no data
    bool has_data = client.get_latest(att);
    assert(!has_data);

    client.stop();
    assert(!client.is_running());

    std::cout << "[PASS] Client lifecycle test passed!" << std::endl;
}

void test_topic_buffer_atomic() {
    pblink::TopicBuffer<pblink::VehicleAttitude> buf;

    assert(!buf.has_data());

    pblink::VehicleAttitude in_msg = pblink::TopicTraits<pblink::VehicleAttitude>::init_default();
    in_msg.q[0] = 1.0f;
    in_msg.q[1] = 0.1f;

    buf.write(in_msg);
    assert(buf.has_data());

    pblink::VehicleAttitude out_msg{};
    uint64_t ts = 0;
    bool ok = buf.read(out_msg, &ts);

    assert(ok);
    assert(ts > 0);
    assert(out_msg.q[0] == 1.0f);
    assert(out_msg.q[1] == 0.1f);

    std::cout << "[PASS] Lock-free TopicBuffer atomic test passed!" << std::endl;
}

int main() {
    std::cout << "Running PBLink Client tests..." << std::endl;
    test_client_lifecycle();
    test_topic_buffer_atomic();
    std::cout << "All Client tests passed!" << std::endl;
    return 0;
}
