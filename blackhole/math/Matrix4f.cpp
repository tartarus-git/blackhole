#include "Matrix4f.h"

#include "math/Vector3f.h"

#include <cmath>

constexpr float& Matrix4f::operator[](int i) { return data[i]; }

Matrix4f& Matrix4f::operator*(Matrix4f& other) {
	Matrix4f result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result[sample(x, y)] = other[sample(y, x)];
		}
	}
	return result;
}

constexpr int Matrix4f::sample(int x, int y) { return y * 4 + x; }

Matrix4f Matrix4f::createRotationMat(float x, float y, float z) {
	Matrix4f result;

	float cosX = cos(x);
	float sinX = sin(x);

	float cosY = cos(y);
	float sinY = sin(y);

	float cosZ = cos(z);
	float sinZ = sin(z);

	// TODO: put xy rotation in here as well. You have to multiply that at the end, so you can just multiply the following with the xy matrix and you're done.
	// Multiplication result of yz * xz rotation matrices.
	result[sample(0, 0)] = cosX;
	result[sample(1, 0)] = 0;
	result[sample(2, 0)] = sinX;
	result[sample(3, 0)] = 0;
	result[sample(0, 1)] = sinY * (-sinX);
	result[sample(1, 1)] = cosY;
	result[sample(2, 1)] = sinY * cosX;
	result[sample(3, 1)] = 0;
	result[sample(0, 2)] = cosY * (-sinX);
	result[sample(1, 2)] = -sinY;
	result[sample(2, 2)] = cosY * cosX;
	result[sample(3, 2)] = 0;
	result[sample(0, 3)] = 0;
	result[sample(1, 3)] = 0;
	result[sample(2, 3)] = 0;
	result[sample(3, 3)] = 1;

	return result;			// TODO: Is this automatically moved to caller.
}

Matrix4f Matrix4f::createRotationMat(Vector3f rot) {
	return createRotationMat(rot.x, rot.y, rot.z);		// TODO: This is automatically moved right?
}
