#pragma once

#include "OpenCLBindingsAndHelpers.h"
#include <string>

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
}
