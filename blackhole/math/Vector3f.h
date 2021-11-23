#pragma once

#include <cmath>

class Vector3f
{
public:
	float x;
	float y;
	float z;

	Vector3f() = default;
	constexpr Vector3f(float x, float y, float z) noexcept : x(x), y(y), z(z) { }		// TODO: Is this the right way to write this?

	constexpr Vector3f operator-() noexcept { return Vector3f(-x, -y, -z); }							// Constexpr functions have to be defined and implemented in the header because or else, the compiler can't calculate compile-time constants from every translation unit (because linking comes after compiling).

	// NOTE: Just so you know, actively returning an r-value reference can prevent copy/move elision, which is even better than moving stuff because no move constructors or assignment
	// operators have to get called, which effectively (if they are explicitly declared) copy a lot of the variables and transfer ownership, which is unnecessesary.
	// In this case it would be fine to return an rvalue ref because Vector3f doesn't explicitly declare any move stuff, but if it did and the compiler didn't optimize our mistake away, using rvalue ref would be harmful.
	// Not using anything here makes it easier for the compiler, even if Vector3f has explicit move stuff, to do copy/move elision.
	// It basically just permutates the lvalue object thats left of the equal sign in the calling code instead of creating one here and copying or moving it, which is way better.

// TODO: Edit the above note so say that returning an l-value reference is actually the best thing to do, which we currently are not doing. We thought we probs shouldn't do it because it could have something to do with pointers, but thats impossible in this situation. It actually makes more sense and signals the intent to optimize more.

	constexpr Vector3f operator+(Vector3f other) noexcept { return Vector3f(x + other.x, y + other.y, z + other.z); }
	constexpr Vector3f operator+=(Vector3f other) noexcept {
		Vector3f result = operator+(other);
		x = result.x;
		y = result.y;
		z = result.z;
		return result;
	}

	Vector3f rotate(Vector3f rot) noexcept {					// TODO: This should be constexpr but the STL doesn't allow it because the trig functions don't satisfy the requirements for constexpr, they modify global variables. If you rewrite the trig functions you can make this constexpr.
		Vector3f result;

		float cosine = cos(rot.y);
		float sine = sin(rot.y);
		result.y = cosine * y - sine * z;
		result.z = sine * y + cosine * z;

		cosine = cos(rot.x);
		sine = sin(rot.x);
		result.x = cosine * x - sine * z;
		result.z = sine * x + cosine * z;

		cosine = cos(rot.z);
		sine = sin(rot.z);
		result.x = cosine * x - sine * y;
		result.y = sine * x + cosine * y;

		return result;
	}
};
