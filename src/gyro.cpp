#include "gyro.h"

#include "alpakka_math.h"
#include "head_track_config.h"
#include "imu.h"
#include "logging.h"
#include "mouse_pipeline.h"

namespace {

double sensitivity_multiplier = 1.0;
double sub_x = 0.0;
double sub_y = 0.0;
double sub_z = 0.0;

uint8_t world_init = 0;
Vector world_top{0.0, 0.0, 1.0};
Vector world_fw{1.0, 0.0, 0.0};
Vector world_right{0.0, 1.0, 0.0};
Vector accel_smooth{0.0, 0.0, 0.0};
bool logging_enabled = true;

double hssnf(double t, double k, double x) {
  const double a = x - (x * k);
  const double b = 1.0 - (x * k * (1.0 / t));
  return a / b;
}

void gyro_accel_correction() {
  Vector accel = imu_read_accel();
  accel.x /= -BIT_14;
  accel.y /= -BIT_14;
  accel.z /= -BIT_14;

  accel_smooth = vector_smooth(accel_smooth, accel, head_track_config::ACCEL_CORRECTION_SMOOTH);

  if (world_init < head_track_config::ACCEL_CORRECTION_SMOOTH) {
    world_top = vector_normalize(vector_invert(accel_smooth));
    world_fw = vector_cross_product(world_top, {1.0, 0.0, 0.0});
    world_right = vector_cross_product(world_fw, world_top);
    ++world_init;
  } else {
    const float rate_fw = static_cast<float>((world_right.z - accel_smooth.x) * head_track_config::ACCEL_CORRECTION_RATE);
    const float rate_r = static_cast<float>((world_fw.z - accel_smooth.y) * head_track_config::ACCEL_CORRECTION_RATE);
    const Vector4 correction_fw = quaternion(world_fw, rate_fw);
    const Vector4 correction_r = quaternion(world_right, -rate_r);
    const Vector4 correction = qmultiply(correction_fw, correction_r);
    world_top = qrotate(correction, world_top);
    world_right = qrotate(correction, world_right);
    world_fw = vector_cross_product(world_top, world_right);
  }
}

}  // namespace

void gyro_reset() {
  sub_x = 0.0;
  sub_y = 0.0;
  sub_z = 0.0;
  world_init = 0;
  world_top = {0.0, 0.0, 1.0};
  world_fw = {1.0, 0.0, 0.0};
  world_right = {0.0, 1.0, 0.0};
  accel_smooth = {0.0, 0.0, 0.0};
}

void gyro_update_sensitivity(double multiplier) {
  sensitivity_multiplier = multiplier;
}

void gyro_set_logging_enabled(bool enabled) {
  logging_enabled = enabled;
}

void gyro_report_incremental(bool emit_mouse) {
  gyro_accel_correction();

  const Vector imu_gyro = imu_read_gyro();
  const Vector imu_accel = imu_read_accel();
  double x = imu_gyro.x * head_track_config::GYRO_SENSITIVITY_X * sensitivity_multiplier;
  double y = imu_gyro.y * head_track_config::GYRO_SENSITIVITY_Y * sensitivity_multiplier;
  double z = imu_gyro.z * head_track_config::GYRO_SENSITIVITY_Z * sensitivity_multiplier;

  constexpr double t = 1.0;
  constexpr double k = 0.5;

  if (x > 0.0 && x < t) x = hssnf(t, k, x);
  else if (x < 0.0 && x > -t) x = -hssnf(t, k, -x);

  if (y > 0.0 && y < t) y = hssnf(t, k, y);
  else if (y < 0.0 && y > -t) y = -hssnf(t, k, -y);

  if (z > 0.0 && z < t) z = hssnf(t, k, z);
  else if (z < 0.0 && z > -t) z = -hssnf(t, k, -z);

  x += sub_x;
  y += sub_y;
  z += sub_z;

  sub_x = modf(x, &x);
  sub_y = modf(y, &y);
  sub_z = modf(z, &z);

  if (logging_enabled) {
    log_runtime_telemetry(imu_gyro, imu_accel, x, y, emit_mouse);
  }

  if (emit_mouse) {
    mouse_pipeline_add_delta(x, y);
  }
}
