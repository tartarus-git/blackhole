#pragma once

#include "OpenCLBindingsAndHelpers.h"
#include <string>

namespace compute {
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue commandQueue;

	cl_int releaseContextVars();

	cl_program program;
	cl_kernel kernel;
	size_t kernelWorkGroupSize;

	cl_int releaseKernelVars();

	cl_int setupDevice(const char* sourceFile, const char* kernelName, std::string& buildLog);
	cl_int releaseDeviceVars();
}
