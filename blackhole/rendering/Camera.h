#pragma once

#include "math/Vector3f.h"

class Camera
{
public:
	Vector3f pos;
	Vector3f rot;
	
	float nearPlane;

	float rotSensitivityX;		// TODO: Maybe put these into a Vector2f or something.
	float rotSensitivityY;

	int requestedDX;
	int requestedDY;
	bool rotRequested = false;

	Camera() = default;
	Camera(Vector3f pos, Vector3f rot, float FOV, float nearPlane) : pos(pos), rot(rot), nearPlane(nearPlane) { }

	void setRotSensitivity(float x, float y);
	void requestRot(int dx, int dy);
	void doRot();
};

