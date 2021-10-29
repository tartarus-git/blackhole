#pragma once

#include "math/Vector3f.h"

class Matrix4f
{
	float data[16];
public:

	Matrix4f() = default;

	constexpr float& operator[](int i);			// Indexing into data.

	Matrix4f& operator*(Matrix4f& other);			// Multiplication with another matrix.

	static Matrix4f createRotationMat(float x, float y, float z);			// Create rotation matrix from rotation values.
	static Matrix4f createRotationMat(Vector3f rot);

	static constexpr int sample(int x, int y);					// Convert 2D index into 1D index for use with data.
};

