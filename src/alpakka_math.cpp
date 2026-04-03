#include "alpakka_math.h"

Vector vector_normalize(Vector v) {
  const double mag_sq = (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
  if (fabs(mag_sq - 1.0) > 0.0001) {
    const double mag = sqrt(mag_sq);
    if (mag == 0.0) {
      return {0.0, 0.0, 0.0};
    }
    return {v.x / mag, v.y / mag, v.z / mag};
  }
  return v;
}

Vector vector_invert(Vector v) {
  return {-v.x, -v.y, -v.z};
}

Vector vector_cross_product(Vector a, Vector b) {
  return {
      (a.y * b.z) - (a.z * b.y),
      (a.z * b.x) - (a.x * b.z),
      (a.x * b.y) - (a.y * b.x),
  };
}

Vector vector_smooth(Vector a, Vector b, float weight) {
  return {
      (a.x * weight + b.x) / (weight + 1.0f),
      (a.y * weight + b.y) / (weight + 1.0f),
      (a.z * weight + b.z) / (weight + 1.0f),
  };
}

Vector4 quaternion(Vector vector, float rotation) {
  vector = vector_normalize(vector);
  const float theta = rotation / 2.0f;
  return {
      static_cast<float>(vector.x * sin(theta)),
      static_cast<float>(vector.y * sin(theta)),
      static_cast<float>(vector.z * sin(theta)),
      cos(theta),
  };
}

Vector4 qmultiply(Vector4 q1, Vector4 q2) {
  return {
      q1.r * q2.x + q1.x * q2.r + q1.y * q2.z - q1.z * q2.y,
      q1.r * q2.y + q1.y * q2.r + q1.z * q2.x - q1.x * q2.z,
      q1.r * q2.z + q1.z * q2.r + q1.x * q2.y - q1.y * q2.x,
      q1.r * q2.r - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
  };
}

Vector4 qconjugate(Vector4 q) {
  return {-q.x, -q.y, -q.z, q.r};
}

Vector qvector(Vector4 q) {
  return vector_normalize({q.x, q.y, q.z});
}

Vector qrotate(Vector4 q1, Vector v) {
  const Vector4 q2{static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z), 0.0f};
  return qvector(qmultiply(qmultiply(q1, q2), qconjugate(q1)));
}
