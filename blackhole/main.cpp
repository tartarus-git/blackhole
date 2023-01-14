#define WINDOW_TITLE "Blackhole"
#define WINDOW_CLASS_NAME "BLACKHOLE_SIM_WINDOW"
#include "window/windowSetup.h"

#include "cl_bindings_and_helpers.h"
#include "OpenCL/specificHelpers.h"

#include "rendering/Renderer.h"

#include "logging/debugOutput.h"

#include "rendering/FrameTimer.h"

#define MAX_FPS 60

#define STARTING_FOV_VALUE 170

#define LOOK_SENSITIVITY_X 0.01f
#define LOOK_SENSITIVITY_Y 0.01f

#define MOVE_SENSITIVITY 1.0f

#define STARTING_LIGHT_SPEED 10
#define STARTING_LIGHT_STEP_AMOUNT 30
#define LIGHT_SPEED_DIFF 0.1f

namespace keys {
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;
	bool space = false;
	bool ctrl = false;

	bool p = false;
	bool o = false;
	bool k = false;
	bool l = false;

	bool n = false;
	bool m = false;
}

Camera camera;
Renderer renderer;

unsigned int halfWindowWidth;
unsigned int halfWindowHeight;
int absHalfWindowWidth;
int absHalfWindowHeight;

bool captureKeyboard = false;
bool captureMouse = false;

bool pendingMouseMove = false;
int mouseMoveX = 0;
int mouseMoveY = 0;
int cachedMouseMoveX = 0;
int cachedMouseMoveY = 0;

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_MOUSEMOVE:
		if (captureMouse) {
			cachedMouseMoveX += LOWORD(lParam) - halfWindowWidth;
			cachedMouseMoveY += HIWORD(lParam) - halfWindowHeight;
			if (!pendingMouseMove) {
				mouseMoveX = cachedMouseMoveX;
				mouseMoveY = cachedMouseMoveY;
				pendingMouseMove = true;
				cachedMouseMoveX = 0;
				cachedMouseMoveY = 0;
				SetCursorPos(absHalfWindowWidth, absHalfWindowHeight);		// Set cursor back to middle of window.
				return 0;
			}
		}
		return 0;
	case WM_KEYDOWN:
		if (captureKeyboard) {
			switch (wParam) {
			case (WPARAM)KeyboardKeys::w: keys::w = true; return 0;
			case (WPARAM)KeyboardKeys::a: keys::a = true; return 0;
			case (WPARAM)KeyboardKeys::s: keys::s = true; return 0;
			case (WPARAM)KeyboardKeys::d: keys::d = true; return 0;
			case (WPARAM)KeyboardKeys::space: keys::space = true; return 0;
			case (WPARAM)KeyboardKeys::ctrl: keys::ctrl = true; return 0;
			case (WPARAM)KeyboardKeys::escape: captureMouse = !captureMouse; return 0;

			case (WPARAM)KeyboardKeys::p: keys::p = true; return 0;
			case (WPARAM)KeyboardKeys::o: keys::o = true; return 0;
			case (WPARAM)KeyboardKeys::k: keys::k = true; return 0;
			case (WPARAM)KeyboardKeys::l: keys::l = true; return 0;

			case (WPARAM)KeyboardKeys::n: keys::n = true; return 0;
			case (WPARAM)KeyboardKeys::m: keys::m = true; return 0;
			}
		}
		return 0;
	case WM_KEYUP:
		if (captureKeyboard) {
			switch (wParam) {
			case (WPARAM)KeyboardKeys::w: keys::w = false; return 0;
			case (WPARAM)KeyboardKeys::a: keys::a = false; return 0;
			case (WPARAM)KeyboardKeys::s: keys::s = false; return 0;
			case (WPARAM)KeyboardKeys::d: keys::d = false; return 0;
			case (WPARAM)KeyboardKeys::space: keys::space = false; return 0;
			case (WPARAM)KeyboardKeys::ctrl: keys::ctrl = false; return 0;

			case (WPARAM)KeyboardKeys::p: keys::p = false; return 0;
			case (WPARAM)KeyboardKeys::o: keys::o = false; return 0;
			case (WPARAM)KeyboardKeys::k: keys::k = false; return 0;
			case (WPARAM)KeyboardKeys::l: keys::l = false; return 0;

			case (WPARAM)KeyboardKeys::n: keys::n = false; return 0;
			case (WPARAM)KeyboardKeys::m: keys::m = false; return 0;
			}
		}
		return 0;
	default:
		if (listenForBoundsChange(uMsg, wParam, lParam)) { return 0; }
		if (listenForExitAttempts(uMsg, wParam, lParam)) { return 0; }
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int windowWidth;
int windowHeight;

int newWindowX;
int newWindowY;
int newWindowWidth;
int newWindowHeight;
bool windowResized = false;

void setWindowSize(unsigned int newWindowWidth, unsigned int newWindowHeight) {
	::newWindowWidth = newWindowWidth;
	::newWindowHeight = newWindowHeight;
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

void setWindow(int newWindowX, int newWindowY, unsigned int newWindowWidth, unsigned int newWindowHeight) {				// This gets triggered once if the first action of you do is to move the window, for the rest of the moves, it doesn't get triggered.
	setWindowSize(newWindowWidth, newWindowHeight);																		// This is practically unavoidable without a little much effort. It's not really bad as long as it's just one time, so I'm going to leave it.
	setWindowPos(newWindowX, newWindowY);
}

void updateWindowSizeVars() {
	windowWidth = newWindowWidth;
	windowHeight = newWindowHeight;
}

#define EXIT_FROM_THREAD POST_THREAD_EXIT; goto releaseAndReturn;

void graphicsLoop() {
	updateWindowSizeVars();
	windowResized = false;

	if (!renderer.init(windowWidth, windowHeight)) { POST_THREAD_EXIT; return; }

	camera = Camera(Vector3f(0, 0, 0), Vector3f(0, 0, 0), STARTING_FOV_VALUE);
	Matrix4f cameraRotMat = Matrix4f::createRotationMat(camera.rot);

	renderer.calculateRayOriginBasis(camera.FOV);

	if (!renderer.loadCameraPos(&camera.pos)) {
		debuglogger::out << debuglogger::error << "failed to load camera pos into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	if (!renderer.loadRayOrigin(windowWidth, windowHeight)) {
		debuglogger::out << debuglogger::error << "failed to load ray origin into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	if (!renderer.loadCameraRotMat(&cameraRotMat)) {
		debuglogger::out << debuglogger::error << "failed to load camera rotation into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	Skybox skybox;
	if (!renderer.loadSkybox(&skybox)) {
		debuglogger::out << debuglogger::error << "failed to load skybox into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	Blackhole blackhole = Blackhole(Vector3f(0, 0, -20), 15, 30);
	if (!renderer.loadBlackholePos(&blackhole.pos)) {
		debuglogger::out << debuglogger::error << "failed to load blackhole position into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	// TODO: The current problem is that when you get close to the blackhole, you see another black hole behind you, which shouldn't happen. You've got to fix your simulation so that it doesn't happen.
	// TODO: Also, the blackhole behind you sometimes has a circular line on it where you can see through it, why is that? Figure that out.

	if (!renderer.loadBlackholeMass(300)) {
		debuglogger::out << debuglogger::error << "failed to load blackhole mass into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	if (!renderer.loadBlackholeBlackRadius(blackhole.blackRadius)) {
		debuglogger::out << debuglogger::error << "failed to load blackhole black radius into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	if (!renderer.loadBlackholeInfluenceRadius(blackhole.influenceRadius)) {
		debuglogger::out << debuglogger::error << "failed to load blackhole influence radius into compute device" << debuglogger::endl;
		POST_THREAD_EXIT; renderer.release(); return;
	}

	HDC finalG = GetDC(hWnd);
	HBITMAP bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
	size_t outputFrame_size = windowWidth * windowHeight * 4;
	char* outputFrame = new char[outputFrame_size];
	HDC g = CreateCompatibleDC(finalG);
	HBITMAP defaultBmp = (HBITMAP)SelectObject(g, bmp);

	captureMouse = true;
	captureKeyboard = true;

	FrameTimer main_frame_timer(MAX_FPS);

	float light_speed = STARTING_LIGHT_SPEED;
	uint32_t light_steps = STARTING_LIGHT_STEP_AMOUNT;
	float prev_light_speed = light_speed;
	uint32_t prev_light_steps = light_steps;

	float fov = STARTING_FOV_VALUE;
	float prev_fov = fov;

	if (!renderer.loadLightSpeed(light_speed)) { debuglogger::out << debuglogger::error << "failed to load light speed" << debuglogger::endl; EXIT_FROM_THREAD; }
	if (!renderer.loadLightStepAmount(light_steps)) { debuglogger::out << debuglogger::error << "failed to load light step amount" << debuglogger::endl; EXIT_FROM_THREAD; }

	while (isAlive) {
		main_frame_timer.mark_frame_start();

		if (!renderer.render(outputFrame)) {
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
			windowResized = false;			// Doing this at beginning leaves space for size event handler to set it to true again while we're recallibrating, which minimizes the chance that the window gets stuck with a drawing surface that doesn't match it's size.
			updateWindowSizeVars();			// NOTE: The chance that something goes wrong with the above is astronomically low and basically zero because size events get fired after resizing is done and user can't start and stop another size move fast enough to trip us up.

			if (!renderer.recallibrateAfterWindowResize(windowWidth, windowHeight)) {
				debuglogger::out << debuglogger::error << "failed to recallibrate renderer after window resize" << debuglogger::endl;
				POST_THREAD_EXIT; renderer.release(); return;
			}

			if (!renderer.loadRayOrigin(windowWidth, windowHeight)) {
				debuglogger::out << debuglogger::error << "failed to load ray origin into compute device" << debuglogger::endl;
				POST_THREAD_EXIT; renderer.release(); return;
			}
			
			// Resize GDI stuff.
			SelectObject(g, defaultBmp);			// Deselect our bmp by replacing it with the defaultBmp that we got from above.
			DeleteObject(bmp);
			bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
			SelectObject(g, bmp);
			delete[] outputFrame;
			outputFrame_size = windowWidth * windowHeight * 4;
			outputFrame = new char[outputFrame_size];

			continue;
		}

		if (pendingMouseMove) {
			camera.rot.x += mouseMoveX * LOOK_SENSITIVITY_X * main_frame_timer.travel_distance_multiplier;			// TODO: Probably prevent this from overflowing by checking if the rot changes break 2pi boundaries and then subtracting by camera.rot.x / 2pi.
			camera.rot.y -= mouseMoveY * LOOK_SENSITIVITY_Y * main_frame_timer.travel_distance_multiplier;
			pendingMouseMove = false;
		}

		Vector3f moveVector = { };
		if (keys::w) { moveVector.z -= MOVE_SENSITIVITY * main_frame_timer.travel_distance_multiplier; }
		if (keys::a) { moveVector.x -= MOVE_SENSITIVITY * main_frame_timer.travel_distance_multiplier; }
		if (keys::s) { moveVector.z += MOVE_SENSITIVITY * main_frame_timer.travel_distance_multiplier; }
		if (keys::d) { moveVector.x += MOVE_SENSITIVITY * main_frame_timer.travel_distance_multiplier; }
		if (keys::space) { moveVector.y += MOVE_SENSITIVITY * main_frame_timer.travel_distance_multiplier; }
		if (keys::ctrl) { moveVector.y -= MOVE_SENSITIVITY * main_frame_timer.travel_distance_multiplier; }
		camera.move(moveVector);			// TODO: Is there really a reason to use custom vector rotation code when you can just pipe the vec through cameraRotMat? Do that.

		if (keys::p) { light_speed += LIGHT_SPEED_DIFF * main_frame_timer.travel_distance_multiplier; }
		if (keys::o) { light_speed -= LIGHT_SPEED_DIFF * main_frame_timer.travel_distance_multiplier; }
		// NOTE: The following could cause UB if 1 * multiplier is larger than a uint32_t can hold after truncation. But that shouldn't happen unless the program is running unbelievably slow.
		// If that's the case, it's unusable anyway, so invoking UB won't really make it worse.
		// But the real reason is that it's very very hard to get the program into a situation like that, and that almost definitely won't happen anyway. If it does though, like I said, it doesn't really matter.
		if (keys::k) { light_steps -= 1 * main_frame_timer.travel_distance_multiplier; }
		if (keys::l) { light_steps += 1 * main_frame_timer.travel_distance_multiplier; }

		if (keys::n) { fov -= 1 * main_frame_timer.travel_distance_multiplier; }
		if (keys::m) { fov += 1 * main_frame_timer.travel_distance_multiplier; }

		if (prev_light_speed != light_speed) {
			if (!renderer.loadLightSpeed(light_speed)) { debuglogger::out << debuglogger::error << "failed to load light speed" << debuglogger::endl; EXIT_FROM_THREAD; }
			prev_light_speed = light_speed;
		}
		if (prev_light_steps != light_steps) {
			if (!renderer.loadLightStepAmount(light_steps)) { debuglogger::out << debuglogger::error << "failed to load light step amount" << debuglogger::endl; EXIT_FROM_THREAD; }
			prev_light_steps = light_steps;
		}

		if (prev_fov != fov) {
			renderer.calculateRayOriginBasis(fov);
			if (!renderer.loadRayOrigin(windowWidth, windowHeight)) { debuglogger::out << debuglogger::error << "failed to load new ray origin (derived from FOV)" << debuglogger::endl; EXIT_FROM_THREAD; }
			prev_fov = fov;
		}

		if (!renderer.loadCameraPos(&camera.pos)) { debuglogger::out << debuglogger::error << "failed to load new camera position" << debuglogger::endl; EXIT_FROM_THREAD; }
		cameraRotMat = Matrix4f::createRotationMat(camera.rot);
		if (!renderer.loadCameraRotMat(&cameraRotMat)) { debuglogger::out << debuglogger::error << "failed to load new camera rotation" << debuglogger::endl; EXIT_FROM_THREAD; }

		main_frame_timer.wait_if_needed();
	}

	releaseAndReturn:
		if (!DeleteDC(g)) { debuglogger::out << debuglogger::error << "failed to delete memory DC (g)" << debuglogger::endl; }
		delete[] outputFrame;
		if (!DeleteObject(bmp)) { debuglogger::out << debuglogger::error << "failed to delete bmp" << debuglogger::endl; }	// This needs to be deleted after it is no longer selected by any DC.
		if (!ReleaseDC(hWnd, finalG)) { debuglogger::out << debuglogger::error << "failed to release window DC (finalG)" << debuglogger::endl; }
		renderer.release();

		// TODO: If this were perfect, this thread would exit with EXIT_FAILURE as well as the main thread if something bad happened, it doesn't yet.
}