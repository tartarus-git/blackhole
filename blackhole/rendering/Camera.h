#pragma once

#include "math/Vector3f.h"
#include "math/Vector4f.h"

class Camera
{
public:
	Vector3f pos;
	Vector3f rot;
	
	float nearPlane;
	float FOV;

	Camera() = default;
	Camera(Vector3f pos, Vector3f rot, float nearPlane, float FOV) : pos(pos), rot(rot), nearPlane(-nearPlane), FOV(FOV) { }			// TODO: Probs put this in implementation file.

	void move(Vector3f move);
};

class DeviceCamera {
public:
	Vector4f pos;								// NOTE: Vector4f's are being used here instead of Vector3f's because OpenCL uses float4 even when you tell it to use float3, which the CPU has to accomodate when transferring data.
	Vector4f rayOrigin;
	float nearPlane;

	DeviceCamera() = default;
	DeviceCamera(Vector3f pos, Vector3f rayOrigin, float nearPlane) : pos(pos), rayOrigin(rayOrigin), nearPlane(nearPlane) { }
};
