#include "Matrix4f.h"

#include <cmath>

#include "math/Vector3f.h"

void Matrix4f::updateRotationMat(Matrix4f& mat, float x, float y, float z) noexcept {
	// TODO: Not implemented for the same reason as below.
}

void Matrix4f::updateRotationMat(Matrix4f& mat, Vector3f rot) noexcept {
	// TODO: Not implemented till you get around to making the ultimate all in one rotation matrix.
}

Matrix4f Matrix4f::createRotationMat(float x, float y, float z) noexcept {
	Matrix4f yzMat = { };

	float cosX = cos(x);
	float sinX = sin(x);

	float cosY = cos(y);
	float sinY = sin(y);

	float cosZ = cos(z);
	float sinZ = sin(z);
	
	yzMat[0][0] = 1;
	yzMat[1][1] = cosY;
	yzMat[1][2] = sinY;
	yzMat[2][1] = -sinY;
	yzMat[2][2] = cosY;
	yzMat[3][3] = 1;

	Matrix4f xzMat = { };
	xzMat[0][0] = cosX;
	xzMat[0][2] = sinX;
	xzMat[2][0] = -sinX;
	xzMat[2][2] = cosX;
	xzMat[1][1] = 1;
	xzMat[3][3] = 1;

	Matrix4f xyMat = { };
	xyMat[0][0] = cosZ;
	xyMat[0][1] = sinZ;
	xyMat[1][0] = -sinZ;
	xyMat[1][1] = cosZ;
	xyMat[2][2] = 1;
	xyMat[3][3] = 1;

	return xyMat * xzMat * yzMat;
}

Matrix4f Matrix4f::createRotationMat(Vector3f rot) noexcept { return createRotationMat(rot.x, rot.y, rot.z); }
