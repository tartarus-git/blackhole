#pragma once

#include "math/Vector3f.h"

class Blackhole
{
public:
	Vector3f pos;
	float blackRadius;
	float influenceRadius;

	Blackhole() = default;
	Blackhole(Vector3f pos, float blackRadius, float influenceRadius) : pos(pos), blackRadius(blackRadius), influenceRadius(influenceRadius) { }
};

