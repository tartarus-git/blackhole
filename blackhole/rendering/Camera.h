#pragma once

#include "math/Vector3f.h"
#include "math/Vector4f.h"

class Camera
{
public:
	Vector3f pos;
	Vector3f rot;
	
	float FOV;

	constexpr Camera() = default;
	constexpr Camera(Vector3f pos, Vector3f rot, float FOV) noexcept : pos(pos), rot(rot), FOV(FOV) { }

	void move(Vector3f move) noexcept;
};
