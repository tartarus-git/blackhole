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
	bool loadCameraRot(Vector3f cameraRot);
	bool loadCamera(Camera camera, unsigned int windowWidth, unsigned int windowHeight);			// Takes apart the given camera. Create rot matrix and transmits that along with Vector3f position coords to the compute device.
	bool loadSkybox(Skybox* skybox);
	bool loadBlackhole(Blackhole* blackhole);

	cl_int render(char* outputFrame);
};

