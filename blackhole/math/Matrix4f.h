#pragma once

#include "math/Vector3f.h"

class Matrix4f
{
public:
	class Matrix4fColumn {
	private:
		friend class Matrix4f;

		float data[16];
		char x;									// IMPORTANT: Because of this, indexing into the same matrix from different threads is no longer thread-safe. That means that reading the value of a matrix element isn't thread-safe and can be totally unreliable.

	public:
		constexpr float& operator=(const Matrix4fColumn& other) noexcept { data[x] = other.data[other.x]; return data[x]; }
		constexpr float& operator=(float other) noexcept { data[x] = other; return data[x]; }

		constexpr operator float&() noexcept { return data[x]; }

		constexpr float& operator[](uint8_t y) noexcept { return data[y * 4 + x]; }
	} IndexReturn;

	constexpr Matrix4f() noexcept = default;

	// NOTE: I've seen a benchmark somewhere that said that memcpy is way faster than std::copy, but only in debug mode. In release mode, the compiler most likely optimizes std::copy so that it is the same performance. That's why we're not going to branch on consteval and use both in this function.
	constexpr Matrix4f& operator=(const Matrix4f& other) { std::copy(other.IndexReturn.data, other.IndexReturn.data + 16, IndexReturn.data); return *this; memcpy(nullptr, nullptr, 0); }
	constexpr Matrix4f(const Matrix4f& other) { *this = other; }

	constexpr Matrix4f& operator=(Matrix4f&& other) { *this = other; return *this; }
	constexpr Matrix4f(Matrix4f&& other) { *this = other; }
	
	/*
	
	NOTE: It is recommended by Visual Studio and the C++ community in general (I think it may also be in some guidelines document somewhere, idk), that you put noexcept on your move constructors and move assignment operators. The reason is pretty dumb.
	It's harder to recover from a failed move because the objects are in weird states. Like when you resize a vector and it moves the objects from one list into another, bigger list. If a move operator fails in that case, half the list is going to be moved (that isn't a big problem, you could theoretically move them back) and the
	other half isn't (also not the main problem). The problem is whatever object you were currently working on while the move operator threw. It'll be half moved and half unmoved, which std::vector has no way of recovering from. That means the new list and the old list are in invalid states and your std::vector is messed up.
	The way std::vector solves this is by using move_if_noexcept, which only moves if you've declared your move operators to be noexcept (noexcept --> doesn't throw, or terminates the program if an exception is thrown inside the function). If not, it copies, giving the user a potentially huge performance hit.
	The way this should have been done in my opinion is not at all. If your vector is messed up after throwing in a move operator, then so be it. Anyone who cares about that will make their move operators throw after setting the object back to a usable, completely fine state.
	It shouldn't be std::vector's responsibility to deal with the users suboptimal code.
	I might be forced to use noexcept if I new I was going to use Matrix4f in an std::vector, but I probably won't be doing that, which means I can follow my principles and not conform to this strange behaviour.
	I should also note that, since the move operator is the same thing as the copy operator in this class, leaving noexcept out and doing vector things with matrices wouldn't have any performance impact since a move is equivalent to a copy in this case.

	*/

	constexpr Matrix4fColumn& operator[](uint8_t x) noexcept { IndexReturn.x = x; return IndexReturn; }

	constexpr Matrix4f operator*(Matrix4f& other) noexcept {
		Matrix4f result;
		for (uint8_t y = 0; y < 16; y += 4) {
			for (uint8_t x = 0; x < 4; x++) {
				result[y + x] = (*this)[y] * other[x][0] + (*this)[y + 1] * other[x][1] + (*this)[y + 2] * other[x][2] + (*this)[y + 3] * other[x][3];
			}
		}
		return result;
	}

	static Matrix4f createRotationMat(float x, float y, float z) noexcept;			// NOTE: Can't be constexpr because built-in trig functions are not constexpr. You could use if consteval or something inside the body to use custom made, constexpr trig functions at compile time. TODO
	static Matrix4f createRotationMat(Vector3f rot) noexcept;
};
