#include "Renderer.h"

#include "OpenCL/OpenCLBindingsAndHelpers.h"
#include "OpenCL/specificHelpers.h"

#include "logging/debugOutput.h"

#define RENDERER_ARGS_START_INDEX 3

void Renderer::setWindowSize(unsigned int windowWidth, unsigned int windowHeight) {
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
}

bool Renderer::updateKernelArgs() {
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX, sizeof(Camera), &this->camera);
	if (err != CL_SUCCESS) { return false; }
	err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX, sizeof(Skybox), &this->skybox);
	if (err != CL_SUCCESS) { return false; }
	err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX, sizeof(Blackhole), &this->blackhole);
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
