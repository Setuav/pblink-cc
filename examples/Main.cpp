#include <pblink/Client.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv) {
    std::string url = "udp://127.0.0.1:14556";
    if (argc > 1) {
        url = argv[1];
    }

    std::cout << "=================================================" << std::endl;
    std::cout << "  pblink-cc Companion SDK Example               " << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout << "Connecting to: " << url << std::endl;

    pblink::Client client(url);

    // 1. Register Callback for Live Telemetry
    client.subscribe<pblink::VehicleAttitude>([](const pblink::VehicleAttitude& att) {
        std::cout << "[CALLBACK] VehicleAttitude -> q: ["
                  << att.q[0] << ", " << att.q[1] << ", "
                  << att.q[2] << ", " << att.q[3] << "]" << std::endl;
    });

    // 2. Start RX Background Thread
    if (!client.start()) {
        std::cerr << "Failed to start PBLink Client!" << std::endl;
        return 1;
    }

    std::cout << "Client running. Requesting topic list..." << std::endl;
    client.request_topic_list();
    client.request_link_status();

    // 3. Main Loop (Polling Latest State @ 10Hz for 10 seconds)
    std::cout << "Listening for telemetry..." << std::endl;
    for (int i = 0; i < 100; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        pblink::VehicleLocalPosition loc_pos{};
        uint64_t ts_us = 0;
        if (client.get_latest(loc_pos, &ts_us)) {
            std::cout << "[POLL] VehicleLocalPosition (ts=" << ts_us << "us) -> "
                      << "x=" << loc_pos.x << ", y=" << loc_pos.y << ", z=" << loc_pos.z << std::endl;
        }

        // Send Offboard Heartbeat
        if (i % 5 == 0) {
            client.send_offboard_control_mode(true, false, false, false, false);
        }
    }

    std::cout << "Stopping PBLink Client..." << std::endl;
    client.stop();
    std::cout << "Finished successfully!" << std::endl;

    return 0;
}
