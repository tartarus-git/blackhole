#pragma once

#include "math/Vector4f.h"

class Blackhole
{
public:
	Vector4f pos;
	float blackRadius;
	float influenceRadius;

	Blackhole() = default;
	Blackhole(Vector4f pos, float blackRadius, float influenceRadius) : pos(pos), blackRadius(blackRadius), influenceRadius(influenceRadius) { }
};

