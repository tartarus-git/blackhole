#include "Matrix4f.h"

#include "math/Vector3f.h"

#include <cmath>

constexpr float& Matrix4f::operator[](int i) { return data[i]; }

Matrix4f& Matrix4f::operator*(Matrix4f& other) {
	Matrix4f result;
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			result[sample(x, y)] = data[sample(x, 0)] * other[sample(0, y)] + data[sample(x, 1)] * other[sample(1, y)] + data[sample(x, 2)] * other[sample(2, y)] + data[sample(x, 3)] * other[sample(3, y)];
		}
	}
	return result;
}

constexpr int Matrix4f::sample(int x, int y) { return x * 4 + y; }

Matrix4f Matrix4f::createRotationMat(float x, float y, float z) {
	Matrix4f result = { };

	float cosX = cos(x);
	float sinX = sin(x);

	float cosY = cos(y);
	float sinY = sin(y);

	float cosZ = cos(z);
	float sinZ = sin(z);

	// TODO: put xy rotation in here as well. You have to multiply that at the end, so you can just multiply the following with the xy matrix and you're done.
	// Multiplication result of yz * xz rotation matrices.
	//result[sample(0, 0)] = cosX;
	//result[sample(1, 0)] = 0;
	//result[sample(2, 0)] = sinX;
	//result[sample(3, 0)] = 0;
	//result[sample(0, 1)] = sinY * (-sinX);
	//result[sample(1, 1)] = cosY;
	//result[sample(2, 1)] = sinY * cosX;
	//result[sample(3, 1)] = 0;
	//result[sample(0, 2)] = cosY * (-sinX);
	//result[sample(1, 2)] = -sinY;
	//result[sample(2, 2)] = cosY * cosX;
	//result[sample(3, 2)] = 0;
	//result[sample(0, 3)] = 0;
	//result[sample(1, 3)] = 0;
	//result[sample(2, 3)] = 0;
	//result[sample(3, 3)] = 1;

	result[sample(0, 0)] = cosX;
	result[sample(0, 2)] = -sinX;
	result[sample(2, 0)] = sinX;
	result[sample(2, 2)] = cosX;
	result[sample(1, 1)] = 1;
	result[sample(3, 3)] = 1;

	Matrix4f result2 = { };

	// TODO: Make constexpr template matrix multiplier for this function.

	result2[sample(1, 1)] = cosY;				// TODO: Thoroughly understand and make sure your matrix code works. You can't be having something in here that you don't understand.
	result2[sample(1, 2)] = -sinY;
	result2[sample(2, 1)] = sinY;
	result2[sample(2, 2)] = cosY;
	result2[sample(0, 0)] = 1;
	result2[sample(3, 3)] = 1;

	Matrix4f result3 = { };

	result3[sample(0, 0)] = cosZ;
	result3[sample(0, 1)] = -sinZ;
	result3[sample(1, 0)] = sinZ;
	result3[sample(1, 1)] = cosZ;
	result3[sample(2, 2)] = 1;
	result3[sample(3, 3)] = 1;

	result = result3 * result * result2;

	return result;
}

Matrix4f Matrix4f::createRotationMat(Vector3f rot) {
	return createRotationMat(rot.x, rot.y, rot.z);
}
