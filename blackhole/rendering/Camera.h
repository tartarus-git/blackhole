#pragma once

#include "math/Vector3f.h"

class Camera
{
public:
	Vector3f pos;
	Vector3f rot;

	Camera(Vector3f pos, Vector3f rot) : pos(pos), rot(rot) { }
};

