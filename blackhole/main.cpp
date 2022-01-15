#define WINDOW_TITLE "Blackhole"
#define WINDOW_CLASS_NAME "BLACKHOLE_SIM_WINDOW"
#include "window/windowSetup.h"

#include "OpenCL/OpenCLBindingsAndHelpers.h"
#include "OpenCL/specificHelpers.h"

#include "rendering/Renderer.h"

#include "logging/debugOutput.h"

#define FOV 120

#define LOOK_SENSITIVITY_X 0.01f
#define LOOK_SENSITIVITY_Y 0.01f

#define MOVE_SENSITIVITY 0.01f

// Virtual key codes.
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44

namespace keys {
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;
}

Camera camera;
Renderer renderer;

unsigned int halfWindowWidth;
unsigned int halfWindowHeight;
int absHalfWindowWidth;
int absHalfWindowHeight;

bool captureKeyboard = false;
bool captureMouse = false;

int mouseTotalX = 0;
int mouseTotalY = 0;
bool pendingMouseMove = false;

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_MOUSEMOVE:
		if (captureMouse) {
			if (!pendingMouseMove) {
				mouseTotalX = LOWORD(lParam) - halfWindowWidth;			// TODO: See if you can find a way to have the windowProc keep adding to mouseTotalX if the graphics loop hasn't processed them yet. Atomics?
				mouseTotalY = HIWORD(lParam) - halfWindowHeight;
				pendingMouseMove = true;
			}
			SetCursorPos(absHalfWindowWidth, absHalfWindowHeight);		// Set cursor back to middle of window.
		}
		return 0;
	case WM_KEYDOWN:
		if (captureKeyboard) {
			switch (wParam) {
			case KEY_W: keys::w = true; return 0;
			case KEY_A: keys::a = true; return 0;
			case KEY_S: keys::s = true; return 0;
			case KEY_D: keys::d = true; return 0;
			case VK_ESCAPE: captureMouse = !captureMouse; return 0;
			}
		}
		return 0;
	case WM_KEYUP:
		if (captureKeyboard) {
			switch (wParam) {
			case KEY_W: keys::w = false; return 0;
			case KEY_A: keys::a = false; return 0;
			case KEY_S: keys::s = false; return 0;
			case KEY_D: keys::d = false; return 0;
			}
		}
	default:
		if (listenForBoundsChange(uMsg, wParam, lParam)) { return 0; }
		if (listenForExitAttempts(uMsg, wParam, lParam)) { return 0; }
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int newWindowX;
int newWindowY;

unsigned int windowWidth;
unsigned int windowHeight;

unsigned int newWindowWidth;
unsigned int newWindowHeight;
bool windowResized = false;
void setWindowSize(unsigned int newWindowWidth, unsigned int newWindowHeight) {								// This gets triggered once if the first action of you do is to move the window, for the rest of the moves, it doesn't get triggered.
	::newWindowWidth = newWindowWidth;																		// This is practically unavoidable without a little much effort. It's not really bad as long as it's just one time, so I'm going to leave it.
	::newWindowHeight = newWindowHeight;																	// TODO: The above comments should go on setWindow, not this function, fix that.
	windowResized = true;
	halfWindowWidth = newWindowWidth / 2;
	halfWindowHeight = newWindowHeight / 2;

	// Calculate screen coords of middle of window for cursor repositioning.
	absHalfWindowWidth = newWindowX + halfWindowWidth;
	absHalfWindowHeight = newWindowY + halfWindowHeight;
}

void setWindowPos(int newWindowX, int newWindowY) {
	::newWindowX = newWindowX;
	::newWindowY = newWindowY;

	// Calculate screen coords of middle of window for cursor repositioning.
	absHalfWindowWidth = newWindowX + halfWindowWidth;
	absHalfWindowHeight = newWindowY + halfWindowHeight;
}

void setWindow(int newWindowX, int newWindowY, unsigned int newWindowWidth, unsigned int newWindowHeight) {
	setWindowSize(newWindowWidth, newWindowHeight);
	setWindowPos(newWindowX, newWindowY);
}

void updateActualWindowSize() {
	windowWidth = newWindowWidth;
	windowHeight = newWindowHeight;
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

void graphicsLoop() {
	updateActualWindowSize();
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
	{
		camera = Camera(Vector3f(0, 0, 0), Vector3f(0, 0, 0), 1, FOV);
		//renderer.calculateRayOriginRawDist(FOV);
		/*if (!renderer.loadNewRayOrigin(windowWidth, windowHeight, camera.nearPlane)) {
			debuglogger::out << debuglogger::error << "failed to load ray origin into compute device" << debuglogger::endl;
			POST_THREAD_EXIT;
			goto OpenCLRelease_all;
		*/}
		if (!renderer.loadCamera(camera)) {
			debuglogger::out << debuglogger::error << "failed to load camera into compute device" << debuglogger::endl;
			POST_THREAD_EXIT;
			goto OpenCLRelease_all;
		}
		Skybox skybox;
		if (!renderer.loadSkybox(&skybox)) {
			debuglogger::out << debuglogger::error << "failed to load skybox into compute device" << debuglogger::endl;
			POST_THREAD_EXIT;
			goto OpenCLRelease_all;
		}
		Blackhole blackhole = Blackhole(Vector3f(0, 0, 0), 10, 20);
		if (!renderer.loadBlackhole(&blackhole)) {
			debuglogger::out << debuglogger::error << "failed to load black hole into compute device" << debuglogger::endl;
			POST_THREAD_EXIT;
			goto OpenCLRelease_all;
		}

		HDC finalG = GetDC(hWnd);
		HBITMAP bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
		size_t outputFrame_size = windowWidth * windowHeight * 4;
		char* outputFrame = new char[outputFrame_size];
		HDC g = CreateCompatibleDC(finalG);
		HBITMAP defaultBmp = (HBITMAP)SelectObject(g, bmp);

		captureMouse = true;
		captureKeyboard = true;

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
				updateActualWindowSize();

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

				// Reposition ray origin.
				if (!renderer.loadNewRayOrigin(windowWidth, windowHeight, camera.nearPlane)) {
					debuglogger::out << debuglogger::error << "failed to update kernel ray origin arg" << debuglogger::endl;
					EXIT_FROM_THREAD;
				}

				// Resize GDI stuff.
				SelectObject(g, defaultBmp);			// Deselect our bmp by replacing it with the defaultBmp that we got from above.
				DeleteObject(bmp);
				bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
				SelectObject(g, bmp);
				delete[] outputFrame;
				outputFrame_size = windowWidth * windowHeight * 4;
				outputFrame = new char[outputFrame_size];

				windowResized = false;
				continue;
			}

			if (pendingMouseMove) {
				camera.rot.x += mouseTotalX * LOOK_SENSITIVITY_X;			// TODO: Probably prevent this from overflowing by checking if the rot changes break 2pi boundaries and then subtracting by camera.rot.x / 2pi.
				camera.rot.y += mouseTotalY * LOOK_SENSITIVITY_Y;
				mouseTotalX = 0;
				mouseTotalY = 0;
				pendingMouseMove = false;
			}

			Vector3f moveVector = { };
			if (keys::w) { moveVector.z -= MOVE_SENSITIVITY; }
			if (keys::a) { moveVector.x -= MOVE_SENSITIVITY; }
			if (keys::s) { moveVector.z += MOVE_SENSITIVITY; }
			if (keys::d) { moveVector.x += MOVE_SENSITIVITY; }
			camera.move(moveVector);

			if (!renderer.loadCamera(camera)) {			// TODO: Make a function that only loads the rot part, which is easy now thanks to the new system.
				debuglogger::out << debuglogger::error << "failed to do requested camera rot" << debuglogger::endl;
				EXIT_FROM_THREAD;
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