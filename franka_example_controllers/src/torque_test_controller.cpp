// Copyright (c) 2021 Franka Emika GmbH
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "torque_test_controller.hpp"
namespace franka_example_controllers {
controller_interface::InterfaceConfiguration TorqueTestController::command_interface_configuration()
    const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

  for (int i = 1; i <= num_joints; ++i) {
    config.names.push_back(arm_id_ + "_joint" + std::to_string(i) + "/effort");
  }
  return config;
}
controller_interface::InterfaceConfiguration TorqueTestController::state_interface_configuration()
    const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  for (int i = 1; i <= num_joints; ++i) {
    config.names.push_back(arm_id_ + "_joint" + std::to_string(i) + "/position");
  }
  return config;
}
controller_interface::return_type TorqueTestController::update() {
  RCLCPP_INFO(rclcpp::get_logger("TorqueTestController"), "updating..");
  for (auto& state_interface : state_interfaces_) {
  }
  for (auto& command_interface : command_interfaces_) {
    command_interface.set_value(0);
  }
  return controller_interface::return_type::OK;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
TorqueTestController::on_configure(const rclcpp_lifecycle::State& previous_state) {
  arm_id_ = node_->get_parameter("arm_id").as_string();
  return LifecycleNodeInterface::on_configure(previous_state);
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
TorqueTestController::on_activate(const rclcpp_lifecycle::State& previous_state) {
  return LifecycleNodeInterface::on_activate(previous_state);
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
TorqueTestController::on_deactivate(const rclcpp_lifecycle::State& previous_state) {
  return LifecycleNodeInterface::on_deactivate(previous_state);
}

controller_interface::return_type TorqueTestController::init(const std::string& controller_name) {
  auto ret = ControllerInterface::init(controller_name);
  if (ret != controller_interface::return_type::OK) {
    return ret;
  }

  try {
    auto_declare<std::string>("arm_id", "panda");
  } catch (const std::exception& e) {
    fprintf(stderr, "Exception thrown during init stage with message: %s \n", e.what());
    return controller_interface::return_type::ERROR;
  }

  return controller_interface::return_type::OK;
}
}  // namespace franka_example_controllers
#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(franka_example_controllers::TorqueTestController,
                       controller_interface::ControllerInterface)