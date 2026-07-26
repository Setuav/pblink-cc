#pragma once

#include <pb_encode.h>
#include <pb_decode.h>

// Nanopb Generated Topic Headers
#include "vehicle_local_position.pb.h"
#include "vehicle_status.pb.h"
#include "vehicle_attitude.pb.h"
#include "vehicle_global_position.pb.h"
#include "battery_status.pb.h"
#include "vehicle_land_detected.pb.h"
#include "home_position.pb.h"
#include "vehicle_command_ack.pb.h"
#include "sensor_combined.pb.h"
#include "sensor_gps.pb.h"
#include "vehicle_angular_velocity.pb.h"
#include "vehicle_acceleration.pb.h"
#include "estimator_status.pb.h"
#include "vehicle_odometry.pb.h"
#include "actuator_outputs.pb.h"
#include "actuator_armed.pb.h"
#include "offboard_control_mode.pb.h"
#include "vehicle_command.pb.h"
#include "manual_control_setpoint.pb.h"
#include "trajectory_setpoint.pb.h"
#include "register_ext_component_request.pb.h"
#include "register_ext_component_reply.pb.h"
#include "unregister_ext_component.pb.h"
#include "arming_check_request.pb.h"
#include "arming_check_reply.pb.h"
#include "onboard_computer_status.pb.h"
#include "vehicle_thrust_setpoint.pb.h"
#include "vehicle_torque_setpoint.pb.h"
#include "vehicle_local_position_setpoint.pb.h"

// Nanopb Generated Custom RPC Headers
#include "subscription_request.pb.h"
#include "time_sync.pb.h"
#include "parameter_request.pb.h"
#include "parameter_set.pb.h"
#include "parameter_value.pb.h"
#include "topic_list_request.pb.h"
#include "topic_list_response.pb.h"
#include "link_status_request.pb.h"
#include "link_status_response.pb.h"

namespace pblink {

// Telemetry & Command Type Aliases
using VehicleLocalPosition          = px4_pblink_msgs_VehicleLocalPosition;
using VehicleStatus                 = px4_pblink_msgs_VehicleStatus;
using VehicleAttitude               = px4_pblink_msgs_VehicleAttitude;
using VehicleGlobalPosition        = px4_pblink_msgs_VehicleGlobalPosition;
using BatteryStatus                 = px4_pblink_msgs_BatteryStatus;
using VehicleLandDetected           = px4_pblink_msgs_VehicleLandDetected;
using HomePosition                  = px4_pblink_msgs_HomePosition;
using VehicleCommandAck             = px4_pblink_msgs_VehicleCommandAck;
using SensorCombined                = px4_pblink_msgs_SensorCombined;
using SensorGps                     = px4_pblink_msgs_SensorGps;
using VehicleAngularVelocity        = px4_pblink_msgs_VehicleAngularVelocity;
using VehicleAcceleration           = px4_pblink_msgs_VehicleAcceleration;
using EstimatorStatus               = px4_pblink_msgs_EstimatorStatus;
using VehicleOdometry               = px4_pblink_msgs_VehicleOdometry;
using ActuatorOutputs               = px4_pblink_msgs_ActuatorOutputs;
using ActuatorArmed                 = px4_pblink_msgs_ActuatorArmed;
using OffboardControlMode           = px4_pblink_msgs_OffboardControlMode;
using VehicleCommand                = px4_pblink_msgs_VehicleCommand;
using ManualControlSetpoint         = px4_pblink_msgs_ManualControlSetpoint;
using TrajectorySetpoint            = px4_pblink_msgs_TrajectorySetpoint;
using RegisterExtComponentRequest   = px4_pblink_msgs_RegisterExtComponentRequest;
using RegisterExtComponentReply     = px4_pblink_msgs_RegisterExtComponentReply;
using UnregisterExtComponent        = px4_pblink_msgs_UnregisterExtComponent;
using ArmingCheckRequest            = px4_pblink_msgs_ArmingCheckRequest;
using ArmingCheckReply              = px4_pblink_msgs_ArmingCheckReply;
using OnboardComputerStatus         = px4_pblink_msgs_OnboardComputerStatus;
using VehicleThrustSetpoint         = px4_pblink_msgs_VehicleThrustSetpoint;
using VehicleTorqueSetpoint         = px4_pblink_msgs_VehicleTorqueSetpoint;
using VehicleLocalPositionSetpoint  = px4_pblink_msgs_VehicleLocalPositionSetpoint;

// RPC Type Aliases
using SubscriptionRequest           = px4_pblink_msgs_SubscriptionRequest;
using TimeSync                      = px4_pblink_msgs_TimeSync;
using ParameterRequest              = px4_pblink_msgs_ParameterRequest;
using ParameterSet                  = px4_pblink_msgs_ParameterSet;
using ParameterValue                = px4_pblink_msgs_ParameterValue;
using TopicListRequest              = px4_pblink_msgs_TopicListRequest;
using TopicListResponse             = px4_pblink_msgs_TopicListResponse;
using LinkStatusRequest             = px4_pblink_msgs_LinkStatusRequest;
using LinkStatusResponse            = px4_pblink_msgs_LinkStatusResponse;

} // namespace pblink
