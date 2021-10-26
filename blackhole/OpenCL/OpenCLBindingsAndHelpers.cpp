#include "OpenCLBindingsAndHelpers.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>																																			// Include these two headers for reading from program source files.
#include <fstream>

HMODULE DLLHandle;

#define CHECK_FUNC_VALIDITY(func) if (!(func)) { FreeLibrary(DLLHandle); return CL_EXT_INIT_FAILURE; }				// Simple helper define that reports error (returns false) if one of the functions doesn't bind correctly. Tries it's best to free OpenCL DLL before exiting.
cl_int initOpenCLBindings() {
	DLLHandle = LoadLibraryA("OpenCL.dll");																													// Load the OpenCL DLL.
	if (!DLLHandle) { return CL_EXT_INIT_FAILURE; }																														// If it doesn't load correctly, fail.

	CHECK_FUNC_VALIDITY(clGetPlatformIDs = (clGetPlatformIDs_func)GetProcAddress(DLLHandle, "clGetPlatformIDs"));											// Go through all of the various functions and bind them (get function pointers to them and store those pointers in variables).
	CHECK_FUNC_VALIDITY(clGetPlatformInfo = (clGetPlatformInfo_func)GetProcAddress(DLLHandle, "clGetPlatformInfo"));										// If any one of these binds fails, everything stops and the whole functions fails.
	CHECK_FUNC_VALIDITY(clGetDeviceIDs = (clGetDeviceIDs_func)GetProcAddress(DLLHandle, "clGetDeviceIDs"));
	CHECK_FUNC_VALIDITY(clGetDeviceInfo = (clGetDeviceInfo_func)GetProcAddress(DLLHandle, "clGetDeviceInfo"));
	CHECK_FUNC_VALIDITY(clCreateContext = (clCreateContext_func)GetProcAddress(DLLHandle, "clCreateContext"));
	CHECK_FUNC_VALIDITY(clCreateCommandQueue = (clCreateCommandQueue_func)GetProcAddress(DLLHandle, "clCreateCommandQueue"));
	CHECK_FUNC_VALIDITY(clCreateProgramWithSource = (clCreateProgramWithSource_func)GetProcAddress(DLLHandle, "clCreateProgramWithSource"));
	CHECK_FUNC_VALIDITY(clBuildProgram = (clBuildProgram_func)GetProcAddress(DLLHandle, "clBuildProgram"));
	CHECK_FUNC_VALIDITY(clGetProgramBuildInfo = (clGetProgramBuildInfo_func)GetProcAddress(DLLHandle, "clGetProgramBuildInfo"));
	CHECK_FUNC_VALIDITY(clCreateKernel = (clCreateKernel_func)GetProcAddress(DLLHandle, "clCreateKernel"));
	CHECK_FUNC_VALIDITY(clCreateBuffer = (clCreateBuffer_func)GetProcAddress(DLLHandle, "clCreateBuffer"));
	CHECK_FUNC_VALIDITY(clCreateImage2D = (clCreateImage2D_func)GetProcAddress(DLLHandle, "clCreateImage2D"));
	CHECK_FUNC_VALIDITY(clSetKernelArg = (clSetKernelArg_func)GetProcAddress(DLLHandle, "clSetKernelArg"));
	CHECK_FUNC_VALIDITY(clGetKernelWorkGroupInfo = (clGetKernelWorkGroupInfo_func)GetProcAddress(DLLHandle, "clGetKernelWorkGroupInfo"));
	CHECK_FUNC_VALIDITY(clEnqueueNDRangeKernel = (clEnqueueNDRangeKernel_func)GetProcAddress(DLLHandle, "clEnqueueNDRangeKernel"));
	CHECK_FUNC_VALIDITY(clFinish = (clFinish_func)GetProcAddress(DLLHandle, "clFinish"));
	CHECK_FUNC_VALIDITY(clEnqueueWriteBuffer = (clEnqueueWriteBuffer_func)GetProcAddress(DLLHandle, "clEnqueueWriteBuffer"));
	CHECK_FUNC_VALIDITY(clEnqueueReadBuffer = (clEnqueueReadBuffer_func)GetProcAddress(DLLHandle, "clEnqueueReadBuffer"));
	CHECK_FUNC_VALIDITY(clEnqueueWriteImage = (clEnqueueWriteImage_func)GetProcAddress(DLLHandle, "clEnqueueWriteImage"));
	CHECK_FUNC_VALIDITY(clEnqueueReadImage = (clEnqueueReadImage_func)GetProcAddress(DLLHandle, "clEnqueueReadImage"));
	CHECK_FUNC_VALIDITY(clReleaseMemObject = (clReleaseMemObject_func)GetProcAddress(DLLHandle, "clReleaseMemObject"));
	CHECK_FUNC_VALIDITY(clReleaseKernel = (clReleaseKernel_func)GetProcAddress(DLLHandle, "clReleaseKernel"));
	CHECK_FUNC_VALIDITY(clReleaseProgram = (clReleaseProgram_func)GetProcAddress(DLLHandle, "clReleaseProgram"));
	CHECK_FUNC_VALIDITY(clReleaseCommandQueue = (clReleaseCommandQueue_func)GetProcAddress(DLLHandle, "clReleaseCommandQueue"));
	CHECK_FUNC_VALIDITY(clReleaseContext = (clReleaseContext_func)GetProcAddress(DLLHandle, "clReleaseContext"));

	return CL_SUCCESS;
}

cl_int freeOpenCLLib() { return FreeLibrary(DLLHandle) ? CL_SUCCESS : CL_EXT_FREE_FAILURE; }

cl_int initOpenCLVarsForBestDevice(const char* targetPlatformVersion, cl_platform_id& bestPlatform, cl_device_id& bestDevice, cl_context& context, cl_command_queue& commandQueue) {
	// Find the best device on the system.

	cl_uint platformCount;																																	// Get the amount of platforms that are available on the system.
	cl_int err = clGetPlatformIDs(0, nullptr, &platformCount);
	if (err != CL_SUCCESS) { return err; }
	if (!platformCount) { return CL_EXT_NO_PLATFORMS_FOUND; }

	cl_platform_id* platforms = new cl_platform_id[platformCount];																							// Get the actual array of platforms after we know how big it is supposed to be.
	err = clGetPlatformIDs(platformCount, platforms, nullptr);
	if (err != CL_SUCCESS) { return err; }

	bool bestPlatformInvalid;																																// Simple algorithm to go through each platform that matches the target version and select the best device out of all possibilities across all platforms.
	size_t bestDeviceMaxWorkGroupSize = 0;
	cl_device_id cachedBestDevice;																															// We cache the best device in this local variable to avoid unnecessarily dereferencing the bestDevice if compiler turns it into pointer.
	for (int i = 0; i < platformCount; i++) {
		cl_platform_id currentPlatform = platforms[i];																										// This is probably done automatically by the compiler, but I like having it here. Makes accessing the platform easier because you don't have to use an add instruction.

		size_t versionStringSize;																															// Get size of version string.
		err = clGetPlatformInfo(currentPlatform, CL_PLATFORM_VERSION, 0, nullptr, &versionStringSize);
		if (err != CL_SUCCESS) { return err; }

		char* buffer = new char[versionStringSize];																											// Get actual version string.
		err = clGetPlatformInfo(currentPlatform, CL_PLATFORM_VERSION, versionStringSize, buffer, nullptr);
		if (err != CL_SUCCESS) { return err; }

		if (!strcmp(buffer, targetPlatformVersion)) {																										// If the platform version matches the target version, continue.
			delete[] buffer;																																// Delete version string buffer as to not waste space.
			bestPlatformInvalid = true;																														// Invalidate bestPlatform so that, if a new best device appears, bestPlatform will be updated as well.

			cl_uint deviceCount;																															// Get the amount of devices on the current platform.
			err = clGetDeviceIDs(currentPlatform, CL_DEVICE_TYPE_GPU, 0, nullptr, &deviceCount);
			if (err != CL_SUCCESS) { return err; }
			if (!deviceCount) { return CL_EXT_NO_DEVICES_FOUND_ON_PLATFORM; }

			cl_device_id* devices = new cl_device_id[deviceCount];																							// Get the actual device IDs of the devices on the current platform.
			err = clGetDeviceIDs(currentPlatform, CL_DEVICE_TYPE_GPU, deviceCount, devices, nullptr);
			if (err != CL_SUCCESS) { return err; }

			for (int j = 0; j < deviceCount; j++) {																											// Go through all the devices on the current platform and see if you can find one that tops the already existing best.
				cl_device_id currentDevice = devices[i];

				size_t deviceMaxWorkGroupSize;																												// Get the theoretical maximum work group size of the current device. This value is how we measure which device has the most computational power, thereby qualifying as the best.
				err = clGetDeviceInfo(currentDevice, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &deviceMaxWorkGroupSize, nullptr);
				if (err != CL_SUCCESS) { return err; }

				if (deviceMaxWorkGroupSize > bestDeviceMaxWorkGroupSize) {																					// If current device is better than best device, set best device to current device.
					bestDeviceMaxWorkGroupSize = deviceMaxWorkGroupSize;
					cachedBestDevice = currentDevice;
					if (bestPlatformInvalid) { bestPlatform = currentPlatform; bestPlatformInvalid = false; }												// Only update the platform if the new best device is on a different platform than the previous best device. Prevents dereferencing (possible) pointers frequently for high device counts.
				}																																			// The reason we don't cache bestPlatform is because it probably won't be more efficient.
			}
			continue;
		}
		delete[] buffer;																																	// Still delete the buffer even if platform doesn't match the target version.
	}
	if (!bestDeviceMaxWorkGroupSize) { return CL_EXT_NO_DEVICES_FOUND; }																					// If no devices were found, stop executing and return an error.
	bestDevice = cachedBestDevice;																															// Update actual bestDevice using the cachedBestDevice variable.

	// Establish other needed vars using the best device on the system.

	context = clCreateContext(nullptr, 1, &bestDevice, nullptr, nullptr, &err);																				// Create context using the best device. Compiler will hopefully optimize the & in case of reference being treated as pointer.
	if (err != CL_SUCCESS) { return err; }

	commandQueue = clCreateCommandQueue(context, cachedBestDevice, 0, &err);																				// Create command queue using the newly created context and the best device. Using cachedBestDevice in case references are pointers.
	if (err != CL_SUCCESS) {
		clReleaseContext(context);											// Errors aren't handled here because it doesn't make a difference if it fails or not.
		return err;
	}

	return CL_SUCCESS;																																		// If no error occurred up until this point, return CL_SUCCESS.
}

char* readFromSourceFile(const char* sourceFile) {
	std::ifstream kernelSourceFile(sourceFile, std::ios::beg);																								// Open the source file.
	if (!kernelSourceFile.is_open()) { return nullptr; }
#pragma push_macro(max)																																		// Count the characters inside the source file, construct large enough buffer, read from file into buffer.
#undef max																																					// The reason for this push, pop and undef stuff is because max is a macro defined in Windows.h header and it interferes with our code.
	kernelSourceFile.ignore(std::numeric_limits<std::streamsize>::max());																					// We shortly undefine it and then pop it's original definition back into the empty slot when we're done with our code.
#pragma pop_macro(max)
	std::streamsize kernelSourceSize = kernelSourceFile.gcount();
	char* kernelSource = new char[kernelSourceSize + 1];
	kernelSourceFile.seekg(0, std::ios::beg);
	kernelSourceFile.read(kernelSource, kernelSourceSize);
	kernelSourceFile.close();
	kernelSource[kernelSourceSize] = '\0';
	return kernelSource;																																	// Returning a raw heap-initialized char array is potentially dangerous. The caller must delete the array.
}

cl_int setupComputeKernel(cl_context context, cl_device_id device, const char* sourceFile, const char* kernelName, cl_program& program, cl_kernel& kernel, size_t& kernelWorkGroupSize, std::string& buildLog) {
	cl_program cachedProgram;																																// We cache the values in case the compiler resorts to pointers instead of references. In that case, caching will be more efficient.
	cl_kernel cachedKernel;
	size_t cachedKernelWorkGroupSize;

	char* kernelSource = readFromSourceFile(sourceFile);																									// Read source code from file.
	if (!kernelSource) { return CL_EXT_FAILED_TO_READ_SOURCE_FILE; }
	cl_int err;
	cachedProgram = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, nullptr, &err);														// Create program with the source code.
	delete[] kernelSource;																																	// Delete kernelSource because readFromSourceFile returned a raw, unsafe pointer that we need to take care of.
	if (!cachedProgram) { return err; }

	err = clBuildProgram(cachedProgram, 0, nullptr, nullptr, nullptr, nullptr);																				// Build program.
	if (err != CL_SUCCESS) {																																// If build fails, return try to return the build log to the user.
		size_t buildLogSize;
		err = clGetProgramBuildInfo(cachedProgram, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &buildLogSize);												// Get size of build log.
		if (err != CL_SUCCESS) {
			clReleaseProgram(cachedProgram);
			return err;
		}
		char* buildLogBuffer = new char[buildLogSize];
		err = clGetProgramBuildInfo(cachedProgram, device, CL_PROGRAM_BUILD_LOG, buildLogSize, buildLogBuffer, nullptr);											// Get actual build log.
		if (err != CL_SUCCESS) {
			delete[] buildLogBuffer;
			clReleaseProgram(cachedProgram);
			return err;
		}																																					// Don't delete build log if it is to be returned to caller because caller needs to use it. The caller is responsible for deleting it.
		clReleaseProgram(cachedProgram);
		buildLog = std::string(buildLogBuffer, buildLogSize - 1);
		return CL_EXT_BUILD_FAILED_WITH_BUILD_LOG;
	}

	cachedKernel = clCreateKernel(cachedProgram, kernelName, &err);																							// Create kernel using a specific kernel function in the program.
	if (!cachedKernel) {
		clReleaseProgram(cachedProgram);
		return err;
	}

	err = clGetKernelWorkGroupInfo(cachedKernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &cachedKernelWorkGroupSize, nullptr);					// Get kernel work group size.
	if (err != CL_SUCCESS) {
		clReleaseKernel(cachedKernel);
		clReleaseProgram(cachedProgram);
		return err;
	}

	size_t computeKernelPreferredWorkGroupSizeMultiple;
	err = clGetKernelWorkGroupInfo(cachedKernel, device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &computeKernelPreferredWorkGroupSizeMultiple, nullptr);					// Get kernel preferred work group size multiple.
	if (err != CL_SUCCESS) {
		clReleaseKernel(cachedKernel);
		clReleaseProgram(cachedProgram);
		return err;
	}

	if (cachedKernelWorkGroupSize > computeKernelPreferredWorkGroupSizeMultiple) { cachedKernelWorkGroupSize -= cachedKernelWorkGroupSize % computeKernelPreferredWorkGroupSizeMultiple; }		// Compute optimal work group size for kernel based on the raw kernel optimum and kernel preferred work group size multiple.

	program = cachedProgram;																																// Update actual values with the cached values.
	kernel = cachedKernel;
	kernelWorkGroupSize = cachedKernelWorkGroupSize;

	return CL_SUCCESS;																																		// Return CL_SUCCESS if we make to this point.
}
