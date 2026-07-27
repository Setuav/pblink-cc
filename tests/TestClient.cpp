#include <pblink/Client.hpp>
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>

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

class MockTransport : public pblink::Transport {
public:
    bool open() override { _is_open = true; return true; }
    void close() override { _is_open = false; }
    bool is_open() const override { return _is_open; }

    ssize_t write(const uint8_t* buffer, size_t length) override {
        (void)buffer;
        return static_cast<ssize_t>(length);
    }

    ssize_t read(uint8_t* buffer, size_t max_length, int timeout_ms) override {
        (void)timeout_ms;
        if (_rx_data.empty()) return 0;
        size_t to_copy = std::min(max_length, _rx_data.size());
        std::memcpy(buffer, _rx_data.data(), to_copy);
        _rx_data.erase(_rx_data.begin(), _rx_data.begin() + to_copy);
        return static_cast<ssize_t>(to_copy);
    }

    void inject_bytes(const uint8_t* data, size_t len) {
        _rx_data.insert(_rx_data.end(), data, data + len);
    }

private:
    bool _is_open{false};
    std::vector<uint8_t> _rx_data;
};

void test_client_mock_rx_and_callback() {
    auto mock_ptr = std::make_unique<MockTransport>();
    auto* mock = mock_ptr.get();

    pblink::Client client(std::move(mock_ptr));

    bool callback_fired = false;
    pblink::VehicleAttitude rcvd_att{};

    client.subscribe<pblink::VehicleAttitude>([&](const pblink::VehicleAttitude& att) {
        callback_fired = true;
        rcvd_att = att;
    });

    client.start();

    // Prepare Nanopb encoded VehicleAttitude message
    pblink::VehicleAttitude att_in = pblink::TopicTraits<pblink::VehicleAttitude>::init_default();
    att_in.q[0] = 0.707f;
    att_in.q[3] = 0.707f;

    uint8_t payload[128];
    pb_ostream_t stream = pb_ostream_from_buffer(payload, sizeof(payload));
    bool enc_ok = pb_encode(&stream, pblink::TopicTraits<pblink::VehicleAttitude>::fields(), &att_in);
    assert(enc_ok);

    uint8_t frame_buf[256];
    size_t frame_len = pblink::Decoder::encode_frame(
        pblink::TopicTraits<pblink::VehicleAttitude>::msg_type_id,
        1,
        payload,
        static_cast<uint16_t>(stream.bytes_written),
        frame_buf,
        sizeof(frame_buf)
    );

    // Inject frame bytes into mock transport
    mock->inject_bytes(frame_buf, frame_len);

    // Wait for rx thread to process frame
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Verify polling API
    pblink::VehicleAttitude polled_att{};
    uint64_t ts = 0;
    bool has_polled = client.get_latest(polled_att, &ts);

    assert(has_polled);
    assert(ts > 0);
    assert(polled_att.q[0] == 0.707f);
    assert(polled_att.q[3] == 0.707f);

    // Verify Callback API
    assert(callback_fired);
    assert(rcvd_att.q[0] == 0.707f);
    assert(rcvd_att.q[3] == 0.707f);

    client.stop();
    std::cout << "[PASS] Client mock RX & callback end-to-end test passed!" << std::endl;
}

int main() {
    std::cout << "Running PBLink Client tests..." << std::endl;
    test_client_lifecycle();
    test_topic_buffer_atomic();
    test_client_mock_rx_and_callback();
    std::cout << "All Client tests passed!" << std::endl;
    return 0;
}
