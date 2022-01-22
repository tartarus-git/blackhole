#pragma once

#include "math/Vector4f.h"

class Blackhole
{
public:
	Vector3f pos;
	float blackRadius;
	float influenceRadius;

	constexpr Blackhole() = default;
	constexpr Blackhole(Vector3f pos, float blackRadius, float influenceRadius) noexcept : pos(pos), blackRadius(blackRadius), influenceRadius(influenceRadius) { }
};

