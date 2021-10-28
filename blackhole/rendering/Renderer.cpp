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

// Figure out a way to just update what you need inside of the camera. Even if it means splitting it open as more than one arg.
bool Renderer::updateKernelCameraArg() {
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX, sizeof(Camera), &(this->camera));// TODO: These parens are unnecessary.
	if (err != CL_SUCCESS) { return false; }
	return true;
}

bool Renderer::updateKernelRayOriginArg() {
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 1, sizeof(Vector3f), &this->rayOrigin);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

// TODO: Obviously move all this around to reflect the order in the header file.
bool Renderer::updateKernelArgs() {			// TODO: This would be a good thing to totally make simpler through a C++ lib for OpenCL that you could make.
	if (!updateKernelCameraArg()) { return false; }
	if (!updateKernelRayOriginArg()) { return false; }
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 2, sizeof(Skybox), &(this->skybox));
	if (err != CL_SUCCESS) { return false; }
	err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 3, sizeof(Blackhole), &this->blackhole);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

void Renderer::calculateRayOriginRawDist(float FOV) {
	rayOriginRawDist = 0.5f / tan(FOV / 180 * constants::pi / 2);		// TODO: Make sure this is as efficient as it can be. Division avoidable?
}

void Renderer::calculateRayOrigin() {
	int refDim;
	if (windowWidth > windowHeight) { refDim = windowHeight; }
	else { refDim = windowWidth; }
	rayOrigin = Vector3f(windowWidth / 2.0f, windowHeight / 2.0f, camera.nearPlane + rayOriginRawDist * refDim);
	// windowWidth / 2.0f is necessary even though we have halfWindowWidth already because this one is more exact because floats, which is necessary for rayOrigin. TODO: Make sure that statement is true.
}

void Renderer::setCameraRotSensitivity(float x, float y) {
	cameraRotSensitivityX = x;
	cameraRotSensitivityY = y;
}

void Renderer::requestCameraRot(int mouseMoveX, int mouseMoveY) {
	if (cameraRotRequested) { return; }							// If there is already one requested, you have to wait until that one is processed before requesting a new one.
	requestedCameraRotX = mouseMoveX * cameraRotSensitivityX;
	requestedCameraRotY = mouseMoveY * cameraRotSensitivityY;
	cameraRotRequested = true;
}

bool Renderer::doRequestedCameraRot() {
	if (cameraRotRequested) {
		camera.rot.x -= requestedCameraRotX;
		camera.rot.y -= requestedCameraRotY;
		if (!updateKernelCameraArg()) { return false; }				// Update the camera in the kernel so that the kernel has new rot information.
		cameraRotRequested = false;
	}

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
