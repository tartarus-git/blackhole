#pragma once

#include "math/Vector3f.h"

class Camera
{
public:
	Vector3f pos;
	Vector3f rot;
	
	float nearPlane;

	Camera() = default;
	Camera(Vector3f pos, Vector3f rot, float FOV, float nearPlane) : pos(pos), rot(rot), nearPlane(nearPlane) { }
};

