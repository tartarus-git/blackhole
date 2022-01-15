#include "Matrix4f.h"

#include <cmath>

#include "math/Vector3f.h"

Matrix4f Matrix4f::createRotationMat(float x, float y, float z) noexcept {
	Matrix4f zyMat = { };

	float cosX = cos(x);
	float sinX = sin(x);

	float cosY = cos(y);
	float sinY = sin(y);

	float cosZ = cos(z);
	float sinZ = sin(z);
	
	zyMat[0][0] = 1;
	zyMat[1][1] = cosY;
	zyMat[1][2] = -sinY;
	zyMat[2][1] = sinY;
	zyMat[2][2] = cosY;
	zyMat[3][3] = 1;

	Matrix4f zxMat = { };
	zxMat[0][0] = cosX;
	zxMat[0][2] = -sinX;
	zxMat[2][0] = sinX;
	zxMat[2][2] = cosX;
	zxMat[1][1] = 1;
	zxMat[3][3] = 1;

	Matrix4f xyMat = { };
	xyMat[0][0] = cosZ;
	xyMat[0][1] = -sinZ;
	xyMat[1][0] = sinZ;
	xyMat[1][1] = cosZ;
	xyMat[2][2] = 1;
	xyMat[3][3] = 1;

	return xyMat * zyMat * zxMat;
}

Matrix4f Matrix4f::createRotationMat(Vector3f rot) noexcept {
	return createRotationMat(rot.x, rot.y, rot.z);
}
