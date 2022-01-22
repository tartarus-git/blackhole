#include "Renderer.h"

#include "OpenCL/OpenCLBindingsAndHelpers.h"
#include "OpenCL/specificHelpers.h"

#include "math/constants.h"

#include "math/Vector3f.h"
#include "rendering/Camera.h"

#include "logging/debugOutput.h"

#define RENDERER_ARGS_START_INDEX 3

Vector3f calculateRayOrigin(float nearPlane, float FOV, unsigned int windowWidth, unsigned int windowHeight) {
	int refDim;
	if (windowWidth > windowHeight) { refDim = windowHeight; }
	else { refDim = windowWidth; }
	return Vector3f(windowWidth / 2.0f, windowHeight / 2.0f, (0.5f / tan(FOV / 180 * constants::pi / 2)) * refDim);			// NOTE: floating-point division is slower than floating point multiplication often because some divisors don't have floating point representations as far as I understand it. x / 10 != x * 0.1f. Does that mean that you could speed something up by doing: x * (1 / 10). The 1/10 would be calculated at compile time and then the resulting multiplication would be faster at run-time. Is that a valid approach? <-- TODO NOTE: BTW, compilers don't do this optimization by default because it slightly changes the value of the result, which, for us, is almost never a problem. You can override this I think by setting some fast-math flag in gcc for example. See about something like that for MSVC
	// windowWidth / 2.0f is necessary even though we have halfWindowWidth already because this one is more exact because floats, which is necessary for rayOrigin. TODO: Make sure that statement is true.
}

bool Renderer::loadCameraRot(Vector3f cameraRot) {
	Matrix4f rot = Matrix4f::createRotationMat(cameraRot.x, cameraRot.y, cameraRot.z);
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 1, 16 * sizeof(float), &rot);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

bool Renderer::loadCamera(Camera camera, unsigned int windowWidth, unsigned int windowHeight) {
	DeviceCamera deviceCamera(camera.pos, calculateRayOrigin(camera.nearPlane, camera.FOV, windowWidth, windowHeight), 0);			// TODO: Reloading the whole device camera everytime the user resizes the display seems kind of stupid. Fix that eventually.
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX, sizeof(DeviceCamera), &deviceCamera);
	if (err != CL_SUCCESS) { return false; }
	return loadCameraRot(camera.rot);
}

bool Renderer::loadSkybox(Skybox* skybox) {
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 2, sizeof(Skybox), skybox);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

bool Renderer::loadBlackhole(Blackhole* blackhole) {
	cl_int err = clSetKernelArg(compute::kernel, RENDERER_ARGS_START_INDEX + 3, sizeof(Blackhole), blackhole);
	if (err != CL_SUCCESS) { return false; }
	return true;
}

// TODO: Obviously move all this around to reflect the order in the header file.

cl_int Renderer::render(char* outputFrame) {
	cl_int err = clEnqueueNDRangeKernel(compute::commandQueue, compute::kernel, 2, nullptr, compute::globalSize, compute::localSize, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) { debuglogger::out << debuglogger::error << "failed to enqueue compute kernel" << debuglogger::endl; return err; }

	err = clEnqueueReadImage(compute::commandQueue, compute::outputFrame, true, compute::frameOrigin, compute::frameRegion, 0, 0, outputFrame, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) { debuglogger::out << debuglogger::error << "failed to read outputFrame_computeImage from compute device" << debuglogger::endl; return err; }

	return CL_SUCCESS;
}
