#include "Renderer.h"

#include "OpenCL/OpenCLBindingsAndHelpers.h"
#include "OpenCL/specificHelpers.h"

#include "math/constants.h"

#include "logging/debugOutput.h"

#define RENDERER_ARGS_START_INDEX 3


// TODO: The loading thing would be cooler if Renderer_args_start_index was a counter or something. Then the order in which the user call these functions would matter, and I wouldn't have to rewrite the numbers everytime I change the kernel args.
bool Renderer::loadCamera(Camera camera) {
	Vector3f pos = camera.pos;
	Matrix4f rot = Matrix4f::createRotationMat(-camera.rot);
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX, sizeof(Vector3f), &pos);
	if (err != CL_SUCCESS) { return false; }
	err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 1, sizeof(Matrix4f), &rot);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

bool Renderer::loadSkybox(Skybox skybox) {
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 3, sizeof(Skybox), &skybox);		// TODO: Using ref arg would probably be better here because we take pointer of it anyway, which the compiler could then more easily optimize.
	if (err != CL_SUCCESS) { return false; }
	return true;
}

bool Renderer::loadBlackhole(Blackhole blackhole) {
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 4, sizeof(Blackhole), &blackhole);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

// TODO: Obviously move all this around to reflect the order in the header file.

void Renderer::calculateRayOriginRawDist(float FOV) {
	rayOriginRawDist = 0.5f / tan(FOV / 180 * constants::pi / 2);		// TODO: Make sure this is as efficient as it can be. Division avoidable?
}

bool Renderer::loadNewRayOrigin(unsigned int windowWidth, unsigned int windowHeight, float nearPlane) {
	int refDim;
	if (windowWidth > windowHeight) { refDim = windowHeight; }
	else { refDim = windowWidth; }
	Vector3f rayOrigin = Vector3f(windowWidth / 2.0f, windowHeight / 2.0f, nearPlane + rayOriginRawDist * refDim);
	// windowWidth / 2.0f is necessary even though we have halfWindowWidth already because this one is more exact because floats, which is necessary for rayOrigin. TODO: Make sure that statement is true.

	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 2, sizeof(Vector3f), &rayOrigin);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

cl_int Renderer::render(char* outputFrame) {
	cl_int err = clEnqueueNDRangeKernel(compute::commandQueue, compute::kernel, 2, nullptr, compute::globalSize, compute::localSize, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to enqueue compute kernel" << debuglogger::endl;
		return err;
	}
	err = clEnqueueReadImage(compute::commandQueue, compute::outputFrame, true, compute::frameOrigin, compute::frameRegion, 0, 0, outputFrame, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to read outputFrame_computeImage from compute device" << debuglogger::endl;
		return err;
	}
	return CL_SUCCESS;
}
