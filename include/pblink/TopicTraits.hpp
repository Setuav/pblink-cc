#pragma once

#include <pblink/Messages.hpp>

namespace pblink {

template <typename T>
struct TopicTraits;

#define DEFINE_TOPIC_TRAITS(TYPE_ALIAS, ID, NANOPB_STRUCT) \
template <> \
struct TopicTraits<TYPE_ALIAS> { \
    static constexpr uint8_t msg_type_id = ID; \
    static const pb_msgdesc_t* fields() { return NANOPB_STRUCT##_fields; } \
    static NANOPB_STRUCT init_default() { return NANOPB_STRUCT##_init_default; } \
};

DEFINE_TOPIC_TRAITS(VehicleLocalPosition,          0x01, px4_pblink_msgs_VehicleLocalPosition)
DEFINE_TOPIC_TRAITS(VehicleStatus,                 0x02, px4_pblink_msgs_VehicleStatus)
DEFINE_TOPIC_TRAITS(VehicleAttitude,               0x03, px4_pblink_msgs_VehicleAttitude)
DEFINE_TOPIC_TRAITS(VehicleGlobalPosition,        0x04, px4_pblink_msgs_VehicleGlobalPosition)
DEFINE_TOPIC_TRAITS(BatteryStatus,                 0x05, px4_pblink_msgs_BatteryStatus)
DEFINE_TOPIC_TRAITS(VehicleLandDetected,           0x06, px4_pblink_msgs_VehicleLandDetected)
DEFINE_TOPIC_TRAITS(HomePosition,                  0x07, px4_pblink_msgs_HomePosition)
DEFINE_TOPIC_TRAITS(VehicleCommandAck,             0x08, px4_pblink_msgs_VehicleCommandAck)
DEFINE_TOPIC_TRAITS(SensorCombined,                0x09, px4_pblink_msgs_SensorCombined)
DEFINE_TOPIC_TRAITS(SensorGps,                     0x0A, px4_pblink_msgs_SensorGps)
DEFINE_TOPIC_TRAITS(VehicleAngularVelocity,        0x0B, px4_pblink_msgs_VehicleAngularVelocity)
DEFINE_TOPIC_TRAITS(VehicleAcceleration,           0x0C, px4_pblink_msgs_VehicleAcceleration)
DEFINE_TOPIC_TRAITS(EstimatorStatus,               0x0D, px4_pblink_msgs_EstimatorStatus)
DEFINE_TOPIC_TRAITS(VehicleOdometry,               0x0E, px4_pblink_msgs_VehicleOdometry)
DEFINE_TOPIC_TRAITS(ActuatorOutputs,               0x0F, px4_pblink_msgs_ActuatorOutputs)
DEFINE_TOPIC_TRAITS(ActuatorArmed,                 0x10, px4_pblink_msgs_ActuatorArmed)
DEFINE_TOPIC_TRAITS(RegisterExtComponentReply,     0x11, px4_pblink_msgs_RegisterExtComponentReply)
DEFINE_TOPIC_TRAITS(ArmingCheckReply,              0x12, px4_pblink_msgs_ArmingCheckReply)
DEFINE_TOPIC_TRAITS(OnboardComputerStatus,         0x13, px4_pblink_msgs_OnboardComputerStatus)
DEFINE_TOPIC_TRAITS(VehicleThrustSetpoint,         0x14, px4_pblink_msgs_VehicleThrustSetpoint)
DEFINE_TOPIC_TRAITS(VehicleTorqueSetpoint,         0x15, px4_pblink_msgs_VehicleTorqueSetpoint)
DEFINE_TOPIC_TRAITS(TopicListResponse,             0x16, px4_pblink_msgs_TopicListResponse)
DEFINE_TOPIC_TRAITS(LinkStatusResponse,            0x17, px4_pblink_msgs_LinkStatusResponse)

DEFINE_TOPIC_TRAITS(OffboardControlMode,           0x80, px4_pblink_msgs_OffboardControlMode)
DEFINE_TOPIC_TRAITS(VehicleCommand,                0x81, px4_pblink_msgs_VehicleCommand)
DEFINE_TOPIC_TRAITS(ManualControlSetpoint,         0x82, px4_pblink_msgs_ManualControlSetpoint)
DEFINE_TOPIC_TRAITS(TrajectorySetpoint,            0x83, px4_pblink_msgs_TrajectorySetpoint)
DEFINE_TOPIC_TRAITS(RegisterExtComponentRequest,   0x84, px4_pblink_msgs_RegisterExtComponentRequest)
DEFINE_TOPIC_TRAITS(UnregisterExtComponent,        0x85, px4_pblink_msgs_UnregisterExtComponent)
DEFINE_TOPIC_TRAITS(ArmingCheckRequest,            0x86, px4_pblink_msgs_ArmingCheckRequest)
DEFINE_TOPIC_TRAITS(VehicleLocalPositionSetpoint,  0x87, px4_pblink_msgs_VehicleLocalPositionSetpoint)
DEFINE_TOPIC_TRAITS(SubscriptionRequest,           0x8B, px4_pblink_msgs_SubscriptionRequest)
DEFINE_TOPIC_TRAITS(ParameterRequest,              0x8C, px4_pblink_msgs_ParameterRequest)
DEFINE_TOPIC_TRAITS(ParameterSet,                  0x8D, px4_pblink_msgs_ParameterSet)
DEFINE_TOPIC_TRAITS(ParameterValue,                0x8E, px4_pblink_msgs_ParameterValue)
DEFINE_TOPIC_TRAITS(TimeSync,                      0x8F, px4_pblink_msgs_TimeSync)
DEFINE_TOPIC_TRAITS(TopicListRequest,              0x90, px4_pblink_msgs_TopicListRequest)
DEFINE_TOPIC_TRAITS(LinkStatusRequest,             0x91, px4_pblink_msgs_LinkStatusRequest)

#undef DEFINE_TOPIC_TRAITS

} // namespace pblink
