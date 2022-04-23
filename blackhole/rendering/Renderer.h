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
	float rayOriginBasis;

	bool init(int windowWidth, int windowHeight) const;

	bool recallibrateAfterWindowResize(int newWindowWidth, int newWindowHeight) const;

	void release() const;

	void calculateRayOriginBasis(float FOV);
	bool loadRayOrigin(int windowWidth, int windowHeight) const;

	bool loadCameraPos(const Vector3f* cameraPos) const;
	bool loadCameraRotMat(const Matrix4f* cameraRotMat) const;

	bool loadSkybox(const Skybox* skybox) const;

	bool loadBlackholePos(const Vector3f* blackholePos) const;
	bool loadBlackholeMass(const float blackholeMass) const;
	bool loadBlackholeBlackRadius(const float blackholeBlackRadius) const;
	bool loadBlackholeInfluenceRadius(const float blackholeInfluenceRadius) const;

	bool render(char* outputFrame) const;
};

