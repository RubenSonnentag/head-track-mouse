#include "mousemovement/runtime_config.h"

namespace mousemovement_runtime {

namespace {

MouseMovementConfig runtime_config{};

}  // namespace

void set_config(const MouseMovementConfig& config) {
  runtime_config = config;
}

const MouseMovementConfig& config() {
  return runtime_config;
}

}  // namespace mousemovement_runtime
