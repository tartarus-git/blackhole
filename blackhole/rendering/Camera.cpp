#include "Camera.h"

void Camera::setRotSensitivity(float x, float y) {
	rotSensitivityX = x;
	rotSensitivityY = y;
}

void Camera::requestRot(int dx, int dy) {
	if (rotRequested) { return; }
	requestedDX = dx;
	requestedDY = dy;
	rotRequested = true;
}

void Camera::doRot() {
	if (rotRequested) {
		rot.x += requestedDX * rotSensitivityX;
		rot.y += requestedDY * rotSensitivityY;
		rotRequested = false;
	}
}

void Camera::requestMove(Vector3f move) { requestedMove = move; moveRequested = true; }

void Camera::move(Vector3f move) { pos += move.rotate(rot); }

void Camera::doMove() { if (moveRequested) { move(requestedMove); moveRequested = false; } }
