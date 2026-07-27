#include <pblink/Client.hpp>
#include <iostream>
#include <cstring>
#include <chrono>

namespace pblink {

Client::Client(const std::string& url)
    : url_(url) {
    transport_ = create_transport_from_url(url_);
    init_buffers();
    decoder_.set_callback([this](const Frame& frame) {
        handle_decoded_frame(frame);
    });
}

Client::Client(std::unique_ptr<Transport> transport)
    : transport_(std::move(transport)) {
    init_buffers();
    decoder_.set_callback([this](const Frame& frame) {
        handle_decoded_frame(frame);
    });
}

Client::~Client() {
    stop();
}

#define INIT_BUFFER(TYPE) get_or_create_buffer<TYPE>(TopicTraits<TYPE>::msg_type_id);

void Client::init_buffers() {
    INIT_BUFFER(VehicleLocalPosition)
    INIT_BUFFER(VehicleStatus)
    INIT_BUFFER(VehicleAttitude)
    INIT_BUFFER(VehicleGlobalPosition)
    INIT_BUFFER(BatteryStatus)
    INIT_BUFFER(VehicleLandDetected)
    INIT_BUFFER(HomePosition)
    INIT_BUFFER(VehicleCommandAck)
    INIT_BUFFER(SensorCombined)
    INIT_BUFFER(SensorGps)
    INIT_BUFFER(VehicleAngularVelocity)
    INIT_BUFFER(VehicleAcceleration)
    INIT_BUFFER(EstimatorStatus)
    INIT_BUFFER(VehicleOdometry)
    INIT_BUFFER(ActuatorOutputs)
    INIT_BUFFER(ActuatorArmed)
    INIT_BUFFER(RegisterExtComponentReply)
    INIT_BUFFER(ArmingCheckReply)
    INIT_BUFFER(OnboardComputerStatus)
    INIT_BUFFER(VehicleThrustSetpoint)
    INIT_BUFFER(VehicleTorqueSetpoint)
    INIT_BUFFER(TopicListResponse)
    INIT_BUFFER(LinkStatusResponse)

    INIT_BUFFER(OffboardControlMode)
    INIT_BUFFER(VehicleCommand)
    INIT_BUFFER(ManualControlSetpoint)
    INIT_BUFFER(TrajectorySetpoint)
    INIT_BUFFER(RegisterExtComponentRequest)
    INIT_BUFFER(UnregisterExtComponent)
    INIT_BUFFER(ArmingCheckRequest)
    INIT_BUFFER(VehicleLocalPositionSetpoint)
    INIT_BUFFER(SubscriptionRequest)
    INIT_BUFFER(ParameterRequest)
    INIT_BUFFER(ParameterSet)
    INIT_BUFFER(ParameterValue)
    INIT_BUFFER(TimeSync)
    INIT_BUFFER(TopicListRequest)
    INIT_BUFFER(LinkStatusRequest)
}

#undef INIT_BUFFER

bool Client::start() {
    if (is_running_) return true;

    if (!transport_) {
        std::cerr << "[pblink] No transport configured!" << std::endl;
        return false;
    }

    if (!transport_->open()) {
        std::cerr << "[pblink] Failed to open transport!" << std::endl;
        return false;
    }

    is_running_ = true;
    rx_thread_ = std::thread(&Client::rx_thread_func, this);
    return true;
}

void Client::stop() {
    if (!is_running_) return;

    is_running_ = false;
    if (rx_thread_.joinable()) {
        rx_thread_.join();
    }

    if (transport_) {
        transport_->close();
    }
}

bool Client::is_running() const {
    return is_running_ && transport_ && transport_->is_open();
}

void Client::rx_thread_func() {
    uint8_t buffer[1024];

    while (is_running_) {
        ssize_t read_bytes = transport_->read(buffer, sizeof(buffer), 10);
        if (read_bytes > 0) {
            decoder_.process_bytes(buffer, read_bytes);
        } else if (read_bytes < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

#define HANDLE_TOPIC(TYPE) \
    case TopicTraits<TYPE>::msg_type_id: { \
        TYPE msg = TopicTraits<TYPE>::init_default(); \
        pb_istream_t stream = pb_istream_from_buffer(frame.payload.data(), frame.payload_len); \
        if (pb_decode(&stream, TopicTraits<TYPE>::fields(), &msg)) { \
            auto* buf = get_or_create_buffer<TYPE>(TopicTraits<TYPE>::msg_type_id); \
            if (buf) buf->write(msg); \
        } \
        break; \
    }

void Client::handle_decoded_frame(const Frame& frame) {
    switch (frame.msg_type_id) {
        HANDLE_TOPIC(VehicleLocalPosition)
        HANDLE_TOPIC(VehicleStatus)
        HANDLE_TOPIC(VehicleAttitude)
        HANDLE_TOPIC(VehicleGlobalPosition)
        HANDLE_TOPIC(BatteryStatus)
        HANDLE_TOPIC(VehicleLandDetected)
        HANDLE_TOPIC(HomePosition)
        HANDLE_TOPIC(VehicleCommandAck)
        HANDLE_TOPIC(SensorCombined)
        HANDLE_TOPIC(SensorGps)
        HANDLE_TOPIC(VehicleAngularVelocity)
        HANDLE_TOPIC(VehicleAcceleration)
        HANDLE_TOPIC(EstimatorStatus)
        HANDLE_TOPIC(VehicleOdometry)
        HANDLE_TOPIC(ActuatorOutputs)
        HANDLE_TOPIC(ActuatorArmed)
        HANDLE_TOPIC(RegisterExtComponentReply)
        HANDLE_TOPIC(ArmingCheckReply)
        HANDLE_TOPIC(OnboardComputerStatus)
        HANDLE_TOPIC(VehicleThrustSetpoint)
        HANDLE_TOPIC(VehicleTorqueSetpoint)
        HANDLE_TOPIC(TopicListResponse)
        HANDLE_TOPIC(LinkStatusResponse)

        HANDLE_TOPIC(OffboardControlMode)
        HANDLE_TOPIC(VehicleCommand)
        HANDLE_TOPIC(ManualControlSetpoint)
        HANDLE_TOPIC(TrajectorySetpoint)
        HANDLE_TOPIC(RegisterExtComponentRequest)
        HANDLE_TOPIC(UnregisterExtComponent)
        HANDLE_TOPIC(ArmingCheckRequest)
        HANDLE_TOPIC(VehicleLocalPositionSetpoint)
        HANDLE_TOPIC(SubscriptionRequest)
        HANDLE_TOPIC(ParameterRequest)
        HANDLE_TOPIC(ParameterSet)
        HANDLE_TOPIC(ParameterValue)
        HANDLE_TOPIC(TimeSync)
        HANDLE_TOPIC(TopicListRequest)
        HANDLE_TOPIC(LinkStatusRequest)
        default:
            break;
    }

    // Trigger raw callbacks
    std::vector<RawCallback> cbs;
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        auto it = callbacks_.find(frame.msg_type_id);
        if (it != callbacks_.end()) {
            cbs = it->second;
        }
    }

    for (const auto& cb : cbs) {
        cb(frame.payload.data(), frame.payload_len);
    }
}

#undef HANDLE_TOPIC

bool Client::send_msg(uint8_t msg_type_id, const pb_msgdesc_t* fields, const void* src_struct) {
    if (!transport_ || !transport_->is_open()) return false;

    uint8_t payload_buf[512];
    pb_ostream_t stream = pb_ostream_from_buffer(payload_buf, sizeof(payload_buf));

    if (!pb_encode(&stream, fields, src_struct)) {
        std::cerr << "[pblink] Nanopb encode failed for msg_type 0x" << std::hex << (int)msg_type_id << std::endl;
        return false;
    }

    uint16_t payload_len = static_cast<uint16_t>(stream.bytes_written);
    uint8_t frame_buf[600];

    size_t frame_len = Decoder::encode_frame(msg_type_id, tx_sequence_++, payload_buf, payload_len, frame_buf, sizeof(frame_buf));
    if (frame_len == 0) return false;

    ssize_t sent = transport_->write(frame_buf, frame_len);
    return sent == static_cast<ssize_t>(frame_len);
}

bool Client::request_topic_list() {
    TopicListRequest req = TopicTraits<TopicListRequest>::init_default();
    return send(req);
}

bool Client::request_link_status() {
    LinkStatusRequest req = TopicTraits<LinkStatusRequest>::init_default();
    return send(req);
}

bool Client::send_offboard_control_mode(bool position, bool velocity, bool acceleration, bool attitude, bool body_rate) {
    OffboardControlMode mode = TopicTraits<OffboardControlMode>::init_default();
    mode.position = position;
    mode.velocity = velocity;
    mode.acceleration = acceleration;
    mode.attitude = attitude;
    mode.body_rate = body_rate;
    return send(mode);
}

bool Client::send_vehicle_command(uint32_t command, float param1, float param2,
                                   float param3, float param4,
                                   double param5, double param6, float param7) {
    VehicleCommand cmd = TopicTraits<VehicleCommand>::init_default();
    cmd.command = command;
    cmd.param1 = param1;
    cmd.param2 = param2;
    cmd.param3 = param3;
    cmd.param4 = param4;
    cmd.param5 = param5;
    cmd.param6 = param6;
    cmd.param7 = param7;
    return send(cmd);
}

} // namespace pblink
