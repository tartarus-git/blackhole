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

	Vector3f rayOrigin;
	void updateRayOrigin();

	bool updateKernelArgs();

	cl_int render(char* outputFrame);
	
	Renderer() = default;
};

