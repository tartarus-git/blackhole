#pragma once

#include "math/Vector3f.h"

class Camera
{
public:
	Vector3f pos;
	Vector3f rot;
	
	float nearPlane;
	float FOV;

	Camera() = default;
	Camera(Vector3f pos, Vector3f rot, float nearPlane, float FOV) : pos(pos), rot(rot), nearPlane(-nearPlane), FOV(FOV) { }			// TODO: Probs put this in implementation file.

	void move(Vector3f move);
};
