#include "specificHelpers.h"

#include "OpenCLBindingsAndHelpers.h"

namespace compute {
	cl_int releaseContextVars() {
		cl_int err = clReleaseCommandQueue(commandQueue);
		if (err != CL_SUCCESS) { return err; }
		err = clReleaseContext(context);
		if (err != CL_SUCCESS) { return err; }
		return CL_SUCCESS;
	}

	cl_int releaseKernelVars() {
		cl_int err = clReleaseKernel(kernel);
		if (err != CL_SUCCESS) { return err; }
		err = clReleaseProgram(program);
		if (err != CL_SUCCESS) { return err; }
		return CL_SUCCESS;
	}

	cl_int setupDevice(const char* sourceFile, const char* kernelName, std::string& buildLog) {
		cl_int err = initOpenCLBindings();
		if (err != CL_SUCCESS) { return err; }

		// TODO: Consider making a system where a platform is selected if it's version is at least the version given here through parameters. Would that work?
		err = initOpenCLVarsForBestDevice("OpenCL 3.0 ", platform, device, context, commandQueue);
		if (err != CL_SUCCESS) { freeOpenCLLib(); return err; }

		err = setupComputeKernel(context, device, "kernels/raytracer.cl", "raytracer", program, kernel, kernelWorkGroupSize, buildLog);
		if (err != CL_SUCCESS) { releaseContextVars(); freeOpenCLLib(); return err; }
	}

	cl_int releaseDeviceVars() {
		cl_int err = releaseKernelVars();
		if (err != CL_SUCCESS) { return err; }
		err = releaseContextVars();
		if (err != CL_SUCCESS) { return err; }
		return CL_SUCCESS;
	}
}