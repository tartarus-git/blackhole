#pragma once

#include "OpenCLBindingsAndHelpers.h"
#include <string>

// This is a collection of data and functions. The functions are here because of generalization.
// The data is here so everything can easily access it (and also because of generalization).
namespace compute {
	inline cl_platform_id platform;
	inline cl_device_id device;
	inline cl_context context;
	inline cl_command_queue commandQueue;

	cl_int releaseContextVars();

	inline cl_program program;
	inline cl_kernel kernel;
	inline size_t kernelWorkGroupSize;

	cl_int releaseKernelVars();

	cl_int setupDevice(const char* sourceFile, const char* kernelName, std::string& buildLog);
	cl_int releaseDeviceVars();

	inline cl_mem outputFrame;

	size_t globalSize[2];
	size_t localSize[2];
	size_t frameOrigin[3];
	size_t frameRegion[3];
}
