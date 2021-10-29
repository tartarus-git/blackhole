#pragma once

class Vector3f
{
public:
	float x;
	float y;
	float z;

	Vector3f() = default;
	constexpr Vector3f(float x, float y, float z) : x(x), y(y), z(z) { }

	constexpr Vector3f operator-() noexcept;
};

