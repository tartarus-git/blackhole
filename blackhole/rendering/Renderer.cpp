#include "Renderer.h"

#include "OpenCL/OpenCLBindingsAndHelpers.h"
#include "OpenCL/specificHelpers.h"

#include "math/constants.h"

#include "logging/debugOutput.h"

#define RENDERER_ARGS_START_INDEX 3

void Renderer::setWindowSize(unsigned int windowWidth, unsigned int windowHeight) {
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
}

bool Renderer::updateKernelArgs() {			// TODO: This would be a good thing to totally make simpler through a C++ lib for OpenCL that you could make.
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX, sizeof(Camera), &(this->camera));// TODO: These parens are unnecessary.
	if (err != CL_SUCCESS) { return false; }
	err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 1, sizeof(Vector3f), &this->rayOrigin);
	if (err != CL_SUCCESS) { return false; }
	err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 2, sizeof(Skybox), &(this->skybox));
	if (err != CL_SUCCESS) { return false; }
	err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 3, sizeof(Blackhole), &this->blackhole);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

void Renderer::updateRayOrigin() {
	float rawDist = 0.5f / tan(camera.FOV / 180 * constants::pi / 2);
	int refDim;
	if (windowWidth > windowHeight) { refDim = windowHeight; }
	else { refDim = windowWidth; }
	rayOrigin = Vector3f(windowWidth / 2.0f, windowHeight / 2.0f, camera.nearPlane + rawDist * refDim);
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
