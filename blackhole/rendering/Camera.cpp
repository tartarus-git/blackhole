#include "Camera.h"

void Camera::move(Vector3f move) noexcept { pos += move.rotate(rot); }