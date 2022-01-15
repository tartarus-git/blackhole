#pragma once

#include "math/Vector3f.h"

class Vector4f {
public:
	float x;
	float y;
	float z;
	float w;

	Vector4f() = default;
	Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
	Vector4f(Vector3f baseVector) : x(baseVector.x), y(baseVector.y), z(baseVector.z), w(1) { }			// TODO: I don't like this conversion. I need it to be really explicit and in the renderer code where we load the camera.
};
