#pragma once

#include "OpenCL/OpenCLBindingsAndHelpers.h"

#include "rendering/Camera.h"
#include "rendering/Skybox.h"

#include "objects/Blackhole.h"

class Renderer
{
public:
	unsigned int windowWidth;
	unsigned int windowHeight;
	void setWindowSize(unsigned int windowWidth, unsigned int windowHeight);

	Camera camera;

	Skybox skybox;

	Blackhole blackhole;

	bool updateKernelArgs();

	cl_int render(char* outputFrame);
	
	Renderer() = default;		// TODO: Figure out how this works.
};

