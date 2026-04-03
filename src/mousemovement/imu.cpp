#include "mousemovement/imu.h"

#include <EEPROM.h>
#include <SPI.h>

#include "logging.h"
#include "mousemovement/head_track_config.h"
#include "mousemovement/runtime_config.h"

namespace {

constexpr uint8_t IMU_WHO_AM_I = 0x0F;
constexpr uint8_t IMU_CTRL1_XL = 0x10;
constexpr uint8_t IMU_CTRL2_G = 0x11;
constexpr uint8_t IMU_CTRL8_XL = 0x17;
constexpr uint8_t IMU_OUTX_L_G = 0x22;
constexpr uint8_t IMU_OUTX_L_XL = 0x28;

constexpr uint8_t IMU_READ = 0x80;
constexpr uint8_t IMU_CTRL1_XL_2G = 0b10100010;
constexpr uint8_t IMU_CTRL8_XL_LP = 0b00000000;
constexpr uint8_t IMU_CTRL2_G_125 = 0b10100010;
constexpr uint8_t IMU_CTRL2_G_500 = 0b10100100;
constexpr uint8_t IMU_WHO_AM_I_EXPECTED = 0x6B;

const SPISettings kSpiSettings(10000000, MSBFIRST, SPI_MODE3);

ImuCalibrationData calibration{
    head_track_config::EEPROM_MAGIC,
    head_track_config::EEPROM_VERSION,
    0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
};
bool calibration_loaded = false;
bool imu_ready = false;

uint8_t cs_pin_for_index(uint8_t index) {
  const auto& pins = mousemovement_runtime::config().pins;
  return index == 0 ? pins.imu_cs0 : pins.imu_cs1;
}

void spi_select(uint8_t cs_pin) {
  SPI.beginTransaction(kSpiSettings);
  digitalWriteFast(cs_pin, LOW);
}

void spi_deselect(uint8_t cs_pin) {
  digitalWriteFast(cs_pin, HIGH);
  SPI.endTransaction();
}

uint8_t imu_read_one(uint8_t cs_pin, uint8_t reg) {
  spi_select(cs_pin);
  SPI.transfer(IMU_READ | reg);
  const uint8_t value = SPI.transfer(0x00);
  spi_deselect(cs_pin);
  return value;
}

void imu_read_many(uint8_t cs_pin, uint8_t reg, uint8_t* buffer, size_t len) {
  spi_select(cs_pin);
  SPI.transfer(IMU_READ | reg);
  for (size_t i = 0; i < len; ++i) {
    buffer[i] = SPI.transfer(0x00);
  }
  spi_deselect(cs_pin);
}

void imu_write_one(uint8_t cs_pin, uint8_t reg, uint8_t value) {
  spi_select(cs_pin);
  SPI.transfer(reg);
  SPI.transfer(value);
  spi_deselect(cs_pin);
}

double* gyro_offset_base(uint8_t index) {
  return index == 0 ? &calibration.offset_gyro_0_x : &calibration.offset_gyro_1_x;
}

double* accel_offset_base(uint8_t index) {
  return index == 0 ? &calibration.offset_accel_0_x : &calibration.offset_accel_1_x;
}

Vector apply_alpakka_v1_gyro_orientation(int16_t raw_x, int16_t raw_y, int16_t raw_z, double* offset) {
  // Sensorlage:
  // raw_x -> links
  // raw_y -> oben
  // raw_z -> Bildschirm
  // Interne Lage:
  // x -> rechts
  // y -> oben
  // z -> Bildschirm
  const double x = -static_cast<double>(raw_x) - offset[0];
  const double y = -static_cast<double>(raw_y) - offset[1];
  const double z = static_cast<double>(raw_z) - offset[2];
  return {x, y, z};
}

Vector apply_alpakka_v1_accel_orientation(int16_t raw_x, int16_t raw_y, int16_t raw_z, double* offset) {
  const double x = -static_cast<double>(raw_x) - offset[0];
  const double y = -static_cast<double>(raw_y) - offset[1];
  const double z = static_cast<double>(raw_z) - offset[2];
  return {x, y, z};
}

Vector imu_read_gyro_bits(uint8_t index) {
  uint8_t buffer[6];
  imu_read_many(cs_pin_for_index(index), IMU_OUTX_L_G, buffer, sizeof(buffer));
  const int16_t raw_x = static_cast<int16_t>((buffer[1] << 8) | buffer[0]);
  const int16_t raw_y = static_cast<int16_t>((buffer[3] << 8) | buffer[2]);
  const int16_t raw_z = static_cast<int16_t>((buffer[5] << 8) | buffer[4]);
  return apply_alpakka_v1_gyro_orientation(raw_x, raw_y, raw_z, gyro_offset_base(index));
}

Vector imu_read_accel_bits(uint8_t index) {
  uint8_t buffer[6];
  imu_read_many(cs_pin_for_index(index), IMU_OUTX_L_XL, buffer, sizeof(buffer));
  const int16_t raw_x = static_cast<int16_t>((buffer[1] << 8) | buffer[0]);
  const int16_t raw_y = static_cast<int16_t>((buffer[3] << 8) | buffer[2]);
  const int16_t raw_z = static_cast<int16_t>((buffer[5] << 8) | buffer[4]);
  return apply_alpakka_v1_accel_orientation(raw_x, raw_y, raw_z, accel_offset_base(index));
}

Vector imu_read_gyro_burst(uint8_t index, uint16_t samples) {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  for (uint16_t i = 0; i < samples; ++i) {
    const Vector sample = imu_read_gyro_bits(index);
    x += sample.x;
    y += sample.y;
    z += sample.z;
  }
  return {x / samples, y / samples, z / samples};
}

void imu_init_single(uint8_t cs_pin, uint8_t gyro_conf) {
  const uint8_t id = imu_read_one(cs_pin, IMU_WHO_AM_I);
  imu_write_one(cs_pin, IMU_CTRL1_XL, IMU_CTRL1_XL_2G);
  imu_write_one(cs_pin, IMU_CTRL8_XL, IMU_CTRL8_XL_LP);
  imu_write_one(cs_pin, IMU_CTRL2_G, gyro_conf);
  const uint8_t xl = imu_read_one(cs_pin, IMU_CTRL1_XL);
  const uint8_t g = imu_read_one(cs_pin, IMU_CTRL2_G);
  const bool ok = id == IMU_WHO_AM_I_EXPECTED;
  (void)xl;
  (void)g;
  (void)ok;
  LOG_MM_IMU("cs=%u whoami=0x%02X xl=0x%02X g=0x%02X status=%s", cs_pin, id, xl, g, ok ? "ok" : "fehler");
}

bool imu_configure_hardware() {
  const auto& pins = mousemovement_runtime::config().pins;
  pinMode(pins.imu_cs0, OUTPUT);
  pinMode(pins.imu_cs1, OUTPUT);
  digitalWriteFast(pins.imu_cs0, HIGH);
  digitalWriteFast(pins.imu_cs1, HIGH);

  SPI.begin();
  imu_init_single(pins.imu_cs0, IMU_CTRL2_G_500);
  imu_init_single(pins.imu_cs1, IMU_CTRL2_G_125);

  const bool ok0 = imu_read_one(pins.imu_cs0, IMU_WHO_AM_I) == IMU_WHO_AM_I_EXPECTED;
  const bool ok1 = imu_read_one(pins.imu_cs1, IMU_WHO_AM_I) == IMU_WHO_AM_I_EXPECTED;
  return ok0 && ok1;
}

void calibration_defaults() {
  calibration = {
      head_track_config::EEPROM_MAGIC, head_track_config::EEPROM_VERSION, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0,                            0.0,                              0.0, 0.0, 0.0, 0.0,
  };
}

void persist_calibration() {
  EEPROM.put(0, calibration);
}

void reset_calibration_runtime() {
  calibration.offset_gyro_0_x = 0.0;
  calibration.offset_gyro_0_y = 0.0;
  calibration.offset_gyro_0_z = 0.0;
  calibration.offset_gyro_1_x = 0.0;
  calibration.offset_gyro_1_y = 0.0;
  calibration.offset_gyro_1_z = 0.0;
  calibration.offset_accel_0_x = 0.0;
  calibration.offset_accel_0_y = 0.0;
  calibration.offset_accel_0_z = 0.0;
  calibration.offset_accel_1_x = 0.0;
  calibration.offset_accel_1_y = 0.0;
  calibration.offset_accel_1_z = 0.0;
}

void imu_calibrate_single(uint8_t index, bool mode_accel, double* x, double* y, double* z) {
  const char* mode_name = mode_accel ? "accel" : "gyro";
  const uint32_t sample_count =
      mode_accel ? head_track_config::CALIBRATION_SAMPLES_ACCEL : head_track_config::CALIBRATION_SAMPLES_GYRO;

  Serial.printf("IMU %u kalibriert %s mit %lu Samples...\r\n", index, mode_name, sample_count);

  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_z = 0.0;

  for (uint32_t i = 0; i < sample_count; ++i) {
    const Vector sample = mode_accel ? imu_read_accel_bits(index) : imu_read_gyro_bits(index);
    sum_x += sample.x;
    sum_y += sample.y;
    sum_z += sample.z;

    if ((i % 5000U) == 0U) {
      yield();
    }
  }

  *x = sum_x / static_cast<double>(sample_count);
  *y = sum_y / static_cast<double>(sample_count);
  *z = sum_z / static_cast<double>(sample_count);
  if (mode_accel) {
    *z -= BIT_14;
  }

  Serial.printf("IMU %u %s Offset x=%.2f y=%.2f z=%.2f\r\n", index, mode_name, *x, *y, *z);
}

}  // namespace

bool imu_init() {
  calibration_defaults();
  LOG_MM_IMU("IMU-Initialisierung startet.");
  imu_ready = imu_configure_hardware();
  imu_load_calibration();
  LOG_MM_IMU("IMU ready=%s calibration_loaded=%s", imu_ready ? "true" : "false", calibration_loaded ? "true" : "false");
  return imu_ready;
}

void imu_load_calibration() {
  ImuCalibrationData stored{};
  EEPROM.get(0, stored);

  if (stored.magic == head_track_config::EEPROM_MAGIC && stored.version == head_track_config::EEPROM_VERSION) {
    calibration = stored;
    calibration_loaded = true;
    LOG_MM_IMU("Kalibrierdaten aus EEPROM geladen.");
  } else {
    calibration_defaults();
    calibration_loaded = false;
    LOG_MM_IMU("Keine gueltigen Kalibrierdaten im EEPROM gefunden.");
  }
}

bool imu_is_ready() {
  return imu_ready;
}

bool imu_has_calibration() {
  return calibration_loaded;
}

bool imu_calibrate() {
  if (!imu_ready) {
    LOG_MM_IMU("Kalibrierung abgebrochen: IMUs nicht bereit.");
    return false;
  }

  calibration_defaults();
  reset_calibration_runtime();
  LOG_MM_IMU("Kalibrierung: Start");

  imu_calibrate_single(0, false, &calibration.offset_gyro_0_x, &calibration.offset_gyro_0_y, &calibration.offset_gyro_0_z);
  imu_calibrate_single(1, false, &calibration.offset_gyro_1_x, &calibration.offset_gyro_1_y, &calibration.offset_gyro_1_z);
  imu_calibrate_single(0, true, &calibration.offset_accel_0_x, &calibration.offset_accel_0_y, &calibration.offset_accel_0_z);
  imu_calibrate_single(1, true, &calibration.offset_accel_1_x, &calibration.offset_accel_1_y, &calibration.offset_accel_1_z);

  calibration.magic = head_track_config::EEPROM_MAGIC;
  calibration.version = head_track_config::EEPROM_VERSION;
  persist_calibration();
  calibration_loaded = true;
  LOG_MM_IMU("Kalibrierung: Abgeschlossen und gespeichert");
  return true;
}

Vector imu_read_gyro() {
  const uint16_t samples_0 = (head_track_config::IMU_TICK_SAMPLES / 8) * 1;
  const uint16_t samples_1 = (head_track_config::IMU_TICK_SAMPLES / 8) * 7;
  const Vector gyro0 = imu_read_gyro_burst(0, samples_0);
  const Vector gyro1 = imu_read_gyro_burst(1, samples_1);
  const double weight = max(fabs(gyro1.x), fabs(gyro1.y)) / 32768.0;
  const double weight_0 = ramp_mid(weight, 0.2);
  const double weight_1 = 1.0 - weight_0;

  return {
      (gyro0.x * weight_0) + (gyro1.x * weight_1 / 4.0),
      (gyro0.y * weight_0) + (gyro1.y * weight_1 / 4.0),
      (gyro0.z * weight_0) + (gyro1.z * weight_1 / 4.0),
  };
}

Vector imu_read_accel() {
  const Vector accel0 = imu_read_accel_bits(0);
  const Vector accel1 = imu_read_accel_bits(1);
  return {
      (accel0.x + accel1.x) / 2.0,
      (accel0.y + accel1.y) / 2.0,
      (accel0.z + accel1.z) / 2.0,
  };
}

void imu_print_status() {
  LOG_MM_IMU("Kalibrierung geladen: %s", calibration_loaded ? "ja" : "nein");
  LOG_MM_IMU("Gyro0 x=%.2f y=%.2f z=%.2f", calibration.offset_gyro_0_x, calibration.offset_gyro_0_y,
             calibration.offset_gyro_0_z);
  LOG_MM_IMU("Gyro1 x=%.2f y=%.2f z=%.2f", calibration.offset_gyro_1_x, calibration.offset_gyro_1_y,
             calibration.offset_gyro_1_z);
}
