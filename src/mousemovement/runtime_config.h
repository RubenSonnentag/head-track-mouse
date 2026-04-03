#pragma once

#include "MouseMovement.h"

namespace mousemovement_runtime {

void set_config(const MouseMovementConfig& config);
const MouseMovementConfig& config();

}  // namespace mousemovement_runtime
