#pragma once

#include <Arduino.h>
#include <math.h>

struct Vector {
  double x;
  double y;
  double z;
};

struct Vector4 {
  float x;
  float y;
  float z;
  float r;
};

constexpr double BIT_18 = 262143.0;
constexpr double BIT_14 = 16383.0;

template <typename T>
constexpr T clamp_value(T value, T low, T high) {
  return value < low ? low : (value > high ? high : value);
}

inline double sign_of(double value) {
  return value >= 0.0 ? 1.0 : -1.0;
}

inline double ramp(double x, double min_value, double max_value) {
  return clamp_value(2.0 * ((x - min_value) / (max_value - min_value)) - 1.0, -1.0, 1.0);
}

inline double ramp_mid(double x, double z) {
  return x < z ? 0.0 : (x > (1.0 - z) ? 1.0 : (x - z) * (1.0 / (1.0 - 2.0 * z)));
}

Vector vector_normalize(Vector v);
Vector vector_invert(Vector v);
Vector vector_cross_product(Vector a, Vector b);
Vector vector_smooth(Vector a, Vector b, float weight);
Vector4 quaternion(Vector vector, float rotation);
Vector4 qmultiply(Vector4 q1, Vector4 q2);
Vector4 qconjugate(Vector4 q);
Vector qrotate(Vector4 q1, Vector v);
Vector qvector(Vector4 q);
