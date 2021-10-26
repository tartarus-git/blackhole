#pragma once

class Vector3f
{
public:
	float x;
	float y;
	float z;

	Vector3f() = default;
	Vector3f(float x, float y, float z) : x(x), y(y), z(z) { }
};

