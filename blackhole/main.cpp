#define WINDOW_TITLE "Blackhole"
#define WINDOW_CLASS_NAME "BLACKHOLE_SIM_WINDOW"
#include "window/windowSetup.h"

#include "OpenCL/OpenCLBindingsAndHelpers.h"
#include "OpenCL/specificHelpers.h"

#include "rendering/Renderer.h"

#include "logging/debugOutput.h"

#define FOV 90

Renderer renderer;

bool captureMouse = false;
LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_MOUSEMOVE:
		if (captureMouse) {
			unsigned int posX = LOWORD(lParam);			// TODO: Understand why this can break on systems with multiple monitors. Doesn't make sense to me just yet.
			unsigned int posY = HIWORD(lParam);
			renderer.camera.rot.x += posX - renderer.windowWidth / 2;
			renderer.camera.rot.y += posY - renderer.windowHeight / 2;
			// TODO: Reset cursor position to middle of client area.
			// TODO: This whole process that we're doing here is not thread safe, your going to have to write most of it again.
		}
	case WM_KEYDOWN:				// TODO: This can break everything if you press it before captureMouse is set to true for the first time, remake the system so that it works.
		if (wParam == VK_ESCAPE) {
			captureMouse = !captureMouse;
		}
	default:
		if (listenForResize(uMsg, wParam, lParam)) { return 0; }
		if (listenForExitAttempts(uMsg, wParam, lParam)) { return 0; }
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

unsigned int windowWidth;
unsigned int windowHeight;

unsigned int newWindowWidth;
unsigned int newWindowHeight;
bool windowResized = false;
void setWindowSize(unsigned int newWindowWidth, unsigned int newWindowHeight) {								// This gets triggered once if the first action of you do is to move the window, for the rest of the moves, it doesn't get triggered.
	::newWindowWidth = newWindowWidth;																		// This is practically unavoidable without a little much effort. It's not really bad as long as it's just one time, so I'm going to leave it.
	::newWindowHeight = newWindowHeight;
	windowResized = true;
}

const cl_image_format computeOutputFrameFormat = { CL_RGBA, CL_UNSIGNED_INT8 };
bool allocateComputeFrameBuffer() {
	cl_int err;
	compute::outputFrame = clCreateImage2D(compute::context, CL_MEM_WRITE_ONLY, &computeOutputFrameFormat, windowWidth, windowHeight, 0, nullptr, &err);
	if (!compute::outputFrame) {
		debuglogger::out << debuglogger::error << "failed to create compute::outputFrame" << debuglogger::endl;
		return true;
	}
	return false;
}

bool reallocateComputeFrameBuffer() {
	cl_int err = clReleaseMemObject(compute::outputFrame);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to release compute::outputFrame" << debuglogger::endl;
		return true;
	}
	if (allocateComputeFrameBuffer()) { return true; }
	return false;
}

bool releaseComputeMemObjects() {
	cl_int err = clReleaseMemObject(compute::outputFrame);
	return err;					// This works because CL_SUCCESS is 0.
}

bool setKernelFramePointer() {
	cl_int err = clSetKernelArg(compute::kernel, 0, sizeof(cl_mem), &compute::outputFrame);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set compute::outputFrame pointer in kernel" << debuglogger::endl;
		return true;
	}
	return false;
}

bool setKernelWindowSizeArgs() {
	cl_int err = clSetKernelArg(compute::kernel, 1, sizeof(unsigned int), &windowWidth);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set windowWidth kernel arg" << debuglogger::endl;
		return true;
	}
	err = clSetKernelArg(compute::kernel, 2, sizeof(unsigned int), &windowHeight);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set windowHeight kernel arg" << debuglogger::endl;
		return true;
	}
	return false;
}

bool setKernelDefaultArgs() {
	if (setKernelFramePointer()) {
		debuglogger::out << debuglogger::error << "failed to set kernel frame pointer" << debuglogger::endl;
		return true;
	}
	if (setKernelWindowSizeArgs()) {
		debuglogger::out << debuglogger::error << "failed to set window size args in kernel" << debuglogger::endl;
		return true;
	}
	return false;
}

void updateKernelInterfaceMetadata() {
	compute::globalSize[0] = windowWidth + (compute::kernelWorkGroupSize - (windowWidth % compute::kernelWorkGroupSize));
	compute::globalSize[1] = windowHeight;
	compute::localSize[0] = compute::kernelWorkGroupSize;
	compute::frameRegion[0] = windowWidth;
	compute::frameRegion[1] = windowHeight;
}

#define EXIT_FROM_THREAD POST_THREAD_EXIT goto releaseEverything;

void graphicsLoop(HWND hWnd) {
	windowWidth = newWindowWidth;
	windowHeight = newWindowHeight;
	windowResized = false;

	std::string buildLog;
	cl_int err = compute::setupDevice("raytracer.cl", "raytracer", buildLog);
	if (err != CL_SUCCESS) {
		debuglogger::out << debuglogger::error << "failed to set up compute device" << debuglogger::endl;
		if (err == CL_EXT_BUILD_FAILED_WITH_BUILD_LOG) {
			debuglogger::out << debuglogger::error << "failed build --> BUILD LOG:" << debuglogger::endl;
			debuglogger::out << buildLog << debuglogger::endl;
		}
		POST_THREAD_EXIT;
		goto OpenCLRelease_freeLib;
	}

	if (allocateComputeFrameBuffer()) {
		debuglogger::out << debuglogger::error << "failed to allocate compute frame buffer" << debuglogger::endl;
		POST_THREAD_EXIT;
		goto OpenCLRelease_releaseDeviceVars;
	}

	if (setKernelDefaultArgs()) {
		debuglogger::out << debuglogger::error << "failed to set default kernel args" << debuglogger::endl;
		POST_THREAD_EXIT;
		goto OpenCLRelease_all;
	}

	compute::localSize[1] = 1;
	compute::frameOrigin[0] = 0;
	compute::frameOrigin[1] = 0;
	compute::frameOrigin[2] = 0;
	compute::frameRegion[2] = 1;
	updateKernelInterfaceMetadata();

	renderer.setWindowSize(windowWidth, windowHeight);

	renderer.camera = Camera(Vector3f(0, 0, 0), Vector3f(0, 0, 0), FOV, 1);
	renderer.updateRayOrigin();
	renderer.skybox = Skybox();
	renderer.blackhole = Blackhole(Vector3f(0, 0, 0), 10, 20);

	captureMouse = true;

	if (!renderer.updateKernelArgs()) {
		debuglogger::out << debuglogger::error << "failed to update renderer kernel args" << debuglogger::endl;
		POST_THREAD_EXIT;
		goto OpenCLRelease_all;
	}

	{
		HDC finalG = GetDC(hWnd);
		HBITMAP bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
		size_t outputFrame_size = windowWidth * windowHeight * 4;
		char* outputFrame = new char[outputFrame_size];
		HDC g = CreateCompatibleDC(finalG);
		HBITMAP defaultBmp = (HBITMAP)SelectObject(g, bmp);

		while (isAlive) {
			cl_int err = renderer.render(outputFrame);
			if (err != CL_SUCCESS) {
				debuglogger::out << debuglogger::error << "failed to render scene" << debuglogger::endl;
				EXIT_FROM_THREAD;
			}

			if (!SetBitmapBits(bmp, outputFrame_size, outputFrame)) {			// TODO: Replace this copy (which is unnecessary), with a direct access to the bitmap bits.
				debuglogger::out << debuglogger::error << "failed to set bmp bits" << debuglogger::endl;
				EXIT_FROM_THREAD;
			}
			if (!BitBlt(finalG, 0, 0, windowWidth, windowHeight, g, 0, 0, SRCCOPY)) {
				debuglogger::out << debuglogger::error << "failed to copy g into finalG" << debuglogger::endl;
				EXIT_FROM_THREAD;
			}

			if (windowResized) {
				windowWidth = newWindowWidth;
				windowHeight = newWindowHeight;

				if (reallocateComputeFrameBuffer()) {
					debuglogger::out << debuglogger::error << "failed to reallocate compute::outputFrame" << debuglogger::endl;
					EXIT_FROM_THREAD;
				}

				if (setKernelFramePointer()) {
					debuglogger::out << debuglogger::error << "failed to update kernel frame pointer" << debuglogger::endl;
					EXIT_FROM_THREAD;
				}

				if (setKernelWindowSizeArgs()) {
					debuglogger::out << debuglogger::error << "failed to update window size args in kernel" << debuglogger::endl;
					EXIT_FROM_THREAD;
				}

				// Resize work group stuff.
				updateKernelInterfaceMetadata();

				// Resize GDI stuff.
				SelectObject(g, defaultBmp);			// Deselect our bmp by replacing it with the defaultBmp that we got from above.
				DeleteObject(bmp);
				bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
				SelectObject(g, bmp);
				delete[] outputFrame;
				outputFrame_size = windowWidth * windowHeight * 4;
				outputFrame = new char[outputFrame_size];

				windowResized = false;
			}
		}

	releaseEverything:
		if (!DeleteDC(g)) { debuglogger::out << debuglogger::error << "failed to delete memory DC (g)" << debuglogger::endl; }
		delete[] outputFrame;
		if (!DeleteObject(bmp)) { debuglogger::out << debuglogger::error << "failed to delete bmp" << debuglogger::endl; }	// This needs to be deleted after it is no longer selected by any DC.
		if (!ReleaseDC(hWnd, finalG)) { debuglogger::out << debuglogger::error << "failed to release window DC (finalG)" << debuglogger::endl; }

		// TODO: If this were perfect, this thread would exit with EXIT_FAILURE as well as the main thread if something bad happened, it doesn't yet.
	}

OpenCLRelease_all:
	if (releaseComputeMemObjects()) { debuglogger::out << debuglogger::error << "failed to release compute memory objects" << debuglogger::endl; }
OpenCLRelease_releaseDeviceVars:
	{
		cl_int err = compute::releaseDeviceVars();
		if (err != CL_SUCCESS) { debuglogger::out << debuglogger::error << "failed to release compute device vars" << debuglogger::endl; }
	}
OpenCLRelease_freeLib:
	{
		cl_int err = freeOpenCLLib();
		if (err != CL_SUCCESS) { debuglogger::out << debuglogger::error << "failed to free OpenCL library" << debuglogger::endl; }
	}
}