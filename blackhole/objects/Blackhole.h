#pragma once

#include "math/Vector4f.h"

class Blackhole
{
public:
	Vector4f pos;
	float blackRadius;
	float influenceRadius;

	constexpr Blackhole() = default;
	constexpr Blackhole(Vector4f pos, float blackRadius, float influenceRadius) noexcept : pos(pos), blackRadius(blackRadius), influenceRadius(influenceRadius) { }
};

