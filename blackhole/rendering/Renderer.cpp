#include "Renderer.h"

#include "cl_bindings_and_helpers.h"
#include "OpenCL/specificHelpers.h"

#include "math/constants.h"

#include "math/Vector3f.h"
#include "rendering/Camera.h"

#include "objects/blackhole.h"

#include "logging/debugOutput.h"

#include <cstdint>

bool allocateOutputFrame(int windowWidth, int windowHeight) {
	cl_int err; compute::outputFrame = clCreateImage2D(compute::context, CL_MEM_WRITE_ONLY, &compute::outputFrameFormat, windowWidth, windowHeight, 0, nullptr, &err);
	if (!compute::outputFrame) { debuglogger::out << debuglogger::error << "failed to allocate compute::outputFrame" << debuglogger::endl; return false; }
	return true;
}

bool setKernelOutputFramePointer() {
	if (clSetKernelArg(compute::kernel, 0, sizeof(cl_mem), &compute::outputFrame) != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set compute::outputFrame pointer in kernel" << debuglogger::endl; return false;
	}
	return true;
}

bool updateDeviceWindowSizeVars(int windowWidth, int windowHeight) {
	if (clSetKernelArg(compute::kernel, 1, sizeof(int), &windowWidth) != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set windowWidth kernel arg" << debuglogger::endl;
		return false;
	}
	if (clSetKernelArg(compute::kernel, 2, sizeof(int), &windowHeight) != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set windowHeight kernel arg" << debuglogger::endl;
		return false;
	}

	float half = windowWidth / 2.0f;
	if (clSetKernelArg(compute::kernel, 3, sizeof(float), &half) != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set halfWindowWidth kernel arg" << debuglogger::endl;
		return false;
	}
	half = windowHeight / 2.0f;
	if (clSetKernelArg(compute::kernel, 4, sizeof(float), &half) != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set halfWindowHeight kernel arg" << debuglogger::endl;
	}

	return true;
}

void updateKernelInterfaceMetadata(int windowWidth, int windowHeight) {
	std::pair<size_t, size_t> local_size = calcSmallestBoundingBox(compute::kernelWorkGroupSize);
	compute::globalSize[0] = windowWidth + (local_size.first - ((windowWidth - 1) % local_size.first + 1));
	compute::globalSize[1] = windowHeight + (local_size.second - ((windowHeight - 1) % local_size.second + 1));
	compute::localSize[0] = local_size.first;
	compute::localSize[1] = local_size.second;
	compute::frameRegion[0] = windowWidth;
	compute::frameRegion[1] = windowHeight;
}

void releaseOpenCLFoundation() {
	if (compute::releaseDeviceVars() != CL_SUCCESS) { debuglogger::out << debuglogger::error << "failed to release compute device vars" << debuglogger::endl; }
	if (freeOpenCLLib() != CL_SUCCESS) { debuglogger::out << debuglogger::error << "failed to free OpenCL library" << debuglogger::endl; }
}

bool releaseOutputFrame() {
	if (clReleaseMemObject(compute::outputFrame) != CL_SUCCESS) { debuglogger::out << debuglogger::error << "failed to release compute::outputFrame" << debuglogger::endl; return false; }
	return true;
}

void Renderer::release() const {
	releaseOutputFrame();
	releaseOpenCLFoundation();
}

bool Renderer::init(int windowWidth, int windowHeight) const {
	std::string buildLog;
	cl_int err = compute::setupDevice("raytracer.cl", "raytracer", buildLog);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set up compute device" << debuglogger::endl;
		if (err == CL_EXT_BUILD_FAILED_WITH_BUILD_LOG) {
			debuglogger::out << debuglogger::error << "failed build --> BUILD LOG:" << debuglogger::endl;
			debuglogger::out << buildLog << debuglogger::endl;
		}
		return false;
	}

	if (!allocateOutputFrame(windowWidth, windowHeight)) { releaseOpenCLFoundation(); return false; }
	
	if (!setKernelOutputFramePointer()) { release(); return false; }

	if (!updateDeviceWindowSizeVars(windowWidth, windowHeight)) { release(); return false; }
	
	// Set a couple of arguments that won't ever change. The rest of these arrays below change so we can't set them here.
	compute::frameOrigin[0] = 0;
	compute::frameOrigin[1] = 0;
	compute::frameOrigin[2] = 0;
	compute::frameRegion[2] = 1;

	// Start the arguments in the previously mentioned arrays that are variable with good starting values.
	updateKernelInterfaceMetadata(windowWidth, windowHeight);

	return true;
}

bool Renderer::recallibrateAfterWindowResize(int newWindowWidth, int newWindowHeight) const {
	if (!releaseOutputFrame()) { return false; }			// If this returns false, the calling code will still try to release the output frame. That seems strange, but it's fine because OpenCL will just return an error and the calling code will move on.
	if (!allocateOutputFrame(newWindowWidth, newWindowHeight)) { return false; }
	if (!setKernelOutputFramePointer()) { return false; }
	if (!updateDeviceWindowSizeVars(newWindowWidth, newWindowHeight)) { return false; }
	updateKernelInterfaceMetadata(newWindowWidth, newWindowHeight);
	return true;
}

#define KERNEL_SCENE_ARGS_START 5

// We're only filling 3/4 of the actual kernel arg since the actual kernel arg is a float4, but the kernel is built such that that last bit of data is allowed to be undefined.
bool Renderer::loadCameraPos(const Vector3f* cameraPos) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START, sizeof(Vector3f), cameraPos) == CL_SUCCESS; }

void Renderer::calculateRayOriginBasis(float FOV) { rayOriginBasis = 0.5f / tan(FOV / 180 * CONSTANTS_PI / 2); }
// NOTE: floating-point division is slower than floating-point multiplication. Often, one can't be converted into the other because some divisors don't have floating point representations as far as I understand it: x / 10 != x * 0.1f. Compilers don't optimize this because, as I said, the numbers differ slightly in the end.
// NOTE: Following this logic, you can speed up floating point division by writing x * (1.0f / 10.0f) instead of x / 10.0f. You can talk the compiler into doing this optimization for you even though it changes the numbers ever so slightly by using some sort of fast-math command line flag, AFAIK, all compilers have one somewhere.
// NOTE: For the above case in calculateRayOriginBasis, the brunt of the divisions should be resolved at compile-time anyway, so it's fine. The one 0.5f / tan thing is unavoidable AFAIK, I can't think of a way to change it so that the division is removed.

bool Renderer::loadRayOrigin(int windowWidth, int windowHeight) const {
	float rayOrigin = rayOriginBasis * (windowWidth > windowHeight ? windowHeight : windowWidth);
	return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 1, sizeof(float), &rayOrigin) == CL_SUCCESS;
}

// TODO: Don't forget about using some sort of typedef for floats to make sure that future expansions to float bit width don't require a bunch of refactoring.

bool Renderer::loadCameraRotMat(const Matrix4f* cameraRotMat) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 2, sizeof(float) * 16, cameraRotMat) == CL_SUCCESS; }

bool Renderer::loadSkybox(const Skybox* skybox) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 3, sizeof(Skybox), skybox) == CL_SUCCESS; }

bool Renderer::loadBlackholePos(const Vector3f* blackholePos) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 4, sizeof(Vector3f), blackholePos) == CL_SUCCESS; }

bool Renderer::loadBlackholeMass(const float blackholeMass) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 5, sizeof(float), &blackholeMass) == CL_SUCCESS; }

bool Renderer::loadBlackholeBlackRadius(const float blackholeBlackRadius) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 6, sizeof(float), &blackholeBlackRadius) == CL_SUCCESS; }

bool Renderer::loadBlackholeInfluenceRadius(const float blackholeInfluenceRadius) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 7, sizeof(float), &blackholeInfluenceRadius) == CL_SUCCESS; }

/*bool Renderer::loadBlackholeDotProducts(const Vector3f& cameraPos, const Blackhole& blackhole) const {
	Vector3f fromBlackhole = cameraPos - blackhole.pos;
	float fromBlackholeLenSquared = fromBlackhole % fromBlackhole;
	float squaredDotProduct = fromBlackholeLenSquared - blackhole.blackRadius * blackhole.blackRadius;			// TODO: remove unnecessary computation. Squaring doesn't have to happen every time.
	if (clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 5, sizeof(float), &squaredDotProduct) != CL_SUCCESS) { return false; }
	squaredDotProduct = fromBlackholeLenSquared - blackhole.influenceRadius * blackhole.influenceRadius;
	return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 6, sizeof(float), &squaredDotProduct) == CL_SUCCESS;
}*/

bool Renderer::loadLightSpeed(float light_speed) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 8, sizeof(light_speed), &light_speed) == CL_SUCCESS; }

bool Renderer::loadLightStepAmount(uint32_t light_steps) const { return clSetKernelArg(compute::kernel, KERNEL_SCENE_ARGS_START + 9, sizeof(light_steps), &light_steps) == CL_SUCCESS; }

// TODO: Obviously move all this around to reflect the order in the header file.

bool Renderer::render(char* outputFrame) const {
	if (clEnqueueNDRangeKernel(compute::commandQueue, compute::kernel, 2, nullptr, compute::globalSize, compute::localSize, 0, nullptr, nullptr) != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to enqueue compute kernel" << debuglogger::endl; return false;
	}
	if (clEnqueueReadImage(compute::commandQueue, compute::outputFrame, true, compute::frameOrigin, compute::frameRegion, 0, 0, outputFrame, 0, nullptr, nullptr) != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to read outputFrame_computeImage from compute device" << debuglogger::endl; return false;
	}
	// IMPORTANT TODO: You completely forgot to use clFinish here, but you have to, or else the CPU could start writing garbage to the screen.
	return true;
}
