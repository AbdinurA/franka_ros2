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

#include <franka_example_controllers/move_to_start_example_controller.hpp>

#include <cassert>
#include <exception>

#include <Eigen/Eigen>
#include <controller_interface/controller_interface.hpp>

namespace franka_example_controllers {

controller_interface::InterfaceConfiguration
MoveToStartExampleController::command_interface_configuration() const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

  for (int i = 1; i <= num_joints; ++i) {
    config.names.push_back(arm_id_ + "_joint" + std::to_string(i) + "/effort");
  }
  return config;
}

controller_interface::InterfaceConfiguration
MoveToStartExampleController::state_interface_configuration() const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  for (int i = 1; i <= num_joints; ++i) {
    config.names.push_back(arm_id_ + "_joint" + std::to_string(i) + "/position");
    config.names.push_back(arm_id_ + "_joint" + std::to_string(i) + "/velocity");
  }
  return config;
}

controller_interface::return_type MoveToStartExampleController::update() {
  updateJointStates();
  auto trajectory_time = this->node_->now() - start_time_;
  auto motion_generator_output = motion_generator_->getDesiredJointPositions(trajectory_time);
  Vector7 q_desired = motion_generator_output.first;
  bool finished = motion_generator_output.second;
  if (not finished) {
    const double kAlpha = 0.99;
    dq_filtered_ = (1 - kAlpha) * dq_filtered_ + kAlpha * dq_;
    Vector7 tau_d_calculated =
        k_gains_.cwiseProduct(q_desired - q_) + d_gains_.cwiseProduct(-dq_filtered_);
    for (int i = 0; i < 7; ++i) {
      command_interfaces_[i].set_value(tau_d_calculated(i));
    }
  } else {
    for (int i = 0; i < 7; ++i) {
      command_interfaces_[i].set_value(0);
    }
  }
  return controller_interface::return_type::OK;
}

controller_interface::return_type MoveToStartExampleController::init(
    const std::string& controller_name) {
  auto ret = ControllerInterface::init(controller_name);
  if (ret != controller_interface::return_type::OK) {
    return ret;
  }
  k_gains_ << 600.0, 600.0, 600.0, 600.0, 250.0, 150.0, 50.0;
  d_gains_ << 25.0, 25.0, 25.0, 25.0, 15.0, 12.5, 7.5;
  q_goal_ << 0, -M_PI_4, 0, -3 * M_PI_4, 0, M_PI_2, M_PI_4;
  try {
    auto_declare<std::string>("arm_id", "panda");
  } catch (const std::exception& e) {
    fprintf(stderr, "Exception thrown during init stage with message: %s \n", e.what());
    return controller_interface::return_type::ERROR;
  }
  return controller_interface::return_type::OK;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
MoveToStartExampleController::on_configure(const rclcpp_lifecycle::State& /*previous_state*/) {
  arm_id_ = node_->get_parameter("arm_id").as_string();
  dq_filtered_.setZero();
  return CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
MoveToStartExampleController::on_activate(const rclcpp_lifecycle::State& /*previous_state*/) {
  updateJointStates();
  motion_generator_ = std::make_unique<MotionGenerator>(0.2, q_, q_goal_);
  start_time_ = this->node_->now();
  return CallbackReturn::SUCCESS;
}

void MoveToStartExampleController::updateJointStates() {
  for (auto i = 0; i < num_joints; ++i) {
    const auto& position_interface = state_interfaces_.at(2 * i);
    const auto& velocity_interface = state_interfaces_.at(2 * i + 1);

    assert(position_interface.get_interface_name() == "position");
    assert(velocity_interface.get_interface_name() == "velocity");

    q_(i) = position_interface.get_value();
    dq_(i) = velocity_interface.get_value();
  }
}
}  // namespace franka_example_controllers
#include "pluginlib/class_list_macros.hpp"
// NOLINTNEXTLINE
PLUGINLIB_EXPORT_CLASS(franka_example_controllers::MoveToStartExampleController,
                       controller_interface::ControllerInterface)