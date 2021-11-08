#pragma once

#include "OpenCL/OpenCLBindingsAndHelpers.h"

#include "math/Vector3f.h"
#include "math/Matrix4f.h"

#include "rendering/Camera.h"
#include "rendering/Skybox.h"

#include "objects/Blackhole.h"

class Renderer
{
public:
	bool loadCamera(Camera camera);			// Takes apart the given camera. Create rot matrix and transmits that along with Vector3f position coords to the compute device.
	bool loadSkybox(Skybox* skybox);
	bool loadBlackhole(Blackhole* blackhole);

	float rayOriginRawDist;
	void calculateRayOriginRawDist(float FOV);			// TODO: Maybe come up with better name or something, idk. Also make it maybe so that you don't always have to call both of these functions. Maybe make another one which combines both or something.

	bool loadNewRayOrigin(unsigned int windowWidth, unsigned int windowHeight, float nearPlane);

	Renderer() = default;

	cl_int render(char* outputFrame);
};

