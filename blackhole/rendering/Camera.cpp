#include "Camera.h"

void Camera::move(Vector3f move) { pos += move.rotate(rot); }