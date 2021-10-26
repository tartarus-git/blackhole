#pragma once

#include "OpenCL/OpenCLBindingsAndHelpers.h"

#include "rendering/Camera.h"
#include "rendering/Skybox.h"

#include "objects/Blackhole.h"

class Renderer
{
public:
	unsigned int windowWidth;				// TODO: This is useless for now, but maybe only for now, which is why it's still here.
	unsigned int windowHeight;				// TODO: I might want to move stuff in this class which depends on the window size.
	void setWindowSize(unsigned int windowWidth, unsigned int windowHeight);

	Camera camera;

	Skybox skybox;

	Blackhole blackhole;

	float rayOriginRawDist;
	Vector3f rayOrigin;
	void calculateRayOriginRawDist(float FOV);			// TODO: Maybe come up with better name or something, idk. Also make it maybe so that you don't always have to call both of these functions. Maybe make another one which combines both or something.
	void calculateRayOrigin();

	Renderer() = default;

	bool updateKernelCameraArg();
	bool updateKernelRayOriginArg();
	bool updateKernelArgs();

	float cameraRotSensitivityX;
	float cameraRotSensitivityY;
	void setCameraRotSensitivity(float x, float y);

	float requestedCameraRotX;
	float requestedCameraRotY;
	bool cameraRotRequested = false;
	void requestCameraRot(int mouseMoveX, int mouseMoveY);

	bool doRequestedCameraRot();

	cl_int render(char* outputFrame);
};

