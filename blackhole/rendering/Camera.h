#pragma once

#include "math/Vector3f.h"

class Camera
{
public:
	Vector3f pos;
	Vector3f rot;
	
	float nearPlane;

	float rotSensitivityX;
	float rotSensitivityY;

	int requestedDX;
	int requestedDY;
	bool rotRequested = false;

	Camera() = default;
	Camera(Vector3f pos, Vector3f rot, float FOV, float nearPlane) : pos(pos), rot(rot), nearPlane(-nearPlane) { }

	void setRotSensitivity(float x, float y);
	void requestRot(int dx, int dy);
	void doRot();

	void move(Vector3f move);
};

