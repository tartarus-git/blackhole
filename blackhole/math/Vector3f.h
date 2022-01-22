#pragma once

#include <cmath>

class Vector3f
{
public:
	float x;
	float y;
	float z;

	constexpr Vector3f() = default;
	constexpr Vector3f(float x, float y, float z) noexcept : x(x), y(y), z(z) { }

	constexpr Vector3f operator-() const noexcept { return Vector3f(-x, -y, -z); }							// Constexpr functions have to be defined and implemented in the header because or else, the compiler can't calculate compile-time constants from every translation unit (because linking comes after compiling).

	// NOTE: Just so you know, actively returning an r-value reference can prevent copy/move elision, which is even better than moving stuff because no move constructors or assignment
	// operators have to get called, which effectively (if they are explicitly declared) copy a lot of the variables and transfer ownership, which is unnecessesary.
	// In this case it would be fine to return an rvalue ref because Vector3f doesn't explicitly declare any move stuff, but if it did and the compiler didn't optimize our mistake away, using rvalue ref would be harmful.
	// Not using anything here makes it easier for the compiler, even if Vector3f has explicit move stuff, to do copy/move elision.
	// It basically just permutates the lvalue object thats left of the equal sign in the calling code instead of creating one here and copying or moving it, which is way better.

	constexpr Vector3f operator+(const Vector3f& other) const noexcept { return Vector3f(x + other.x, y + other.y, z + other.z); }
	constexpr Vector3f& operator+=(const Vector3f& other) noexcept { *this = *this + other; return *this; }
	constexpr Vector3f operator-(const Vector3f& other) const noexcept { return Vector3f(x - other.x, y - other.y, z - other.z); }

	constexpr float operator%(const Vector3f& other) const noexcept { return x * other.x + y * other.y + z * other.z; }

	Vector3f rotate(const Vector3f& other) const noexcept;
};
