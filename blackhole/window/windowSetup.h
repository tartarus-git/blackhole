#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>									// For GET_X_LPARAM and GET_Y_LPARAM

#include "logging/debugOutput.h"

#include <thread>

enum class KeyboardKeys {
	w = 0x57,
	a = 0x41,
	s = 0x53,
	d = 0x44,
	space = VK_SPACE,
	ctrl = VK_CONTROL,
	escape = VK_ESCAPE
};

HWND hWnd;

void setWindowPos(int newPosX, int newPosY);
void setWindowSize(unsigned int newWidth, unsigned int newHeight);
void setWindow(int newPosX, int newPosY, unsigned int newWidth, unsigned int newHeight);

int tempWindowPosX;
int tempWindowPosY;
bool tempWindowPosUpdated = false;

bool windowMaximized = false;
unsigned int tempWindowWidth;
unsigned int tempWindowHeight;
bool tempWindowSizesUpdated = false;
bool listenForBoundsChange(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_MOVE:
		tempWindowPosX = GET_X_LPARAM(lParam);
		tempWindowPosY = GET_Y_LPARAM(lParam);
		tempWindowPosUpdated = true;
		return true;
	case WM_SIZE:
		switch (wParam) {
		case SIZE_RESTORED:
			if (windowMaximized) {
				setWindowSize(LOWORD(lParam), HIWORD(lParam));
				windowMaximized = false;
				return true;
			}
			tempWindowWidth = LOWORD(lParam);
			tempWindowHeight = HIWORD(lParam);
			tempWindowSizesUpdated = true;
			return true;
		case SIZE_MAXIMIZED:
			POINT pos = { 0, 0 };
			ClientToScreen(hWnd, &pos);
			setWindow(pos.x, pos.y, LOWORD(lParam), HIWORD(lParam));
			windowMaximized = true;
			tempWindowSizesUpdated = false;
			return true;
		}
		return false;
	case WM_EXITSIZEMOVE:
		if (tempWindowPosUpdated) {
			if (tempWindowSizesUpdated) {
				setWindow(tempWindowPosX, tempWindowPosY, tempWindowWidth, tempWindowHeight);
				tempWindowPosUpdated = false;
				tempWindowSizesUpdated = false;
				return true;
			}
			setWindowPos(tempWindowPosX, tempWindowPosY);
			tempWindowPosUpdated = false;
			return true;
		}
		if (tempWindowSizesUpdated) {
			if (tempWindowPosUpdated) {
				setWindow(tempWindowPosX, tempWindowPosY, tempWindowWidth, tempWindowHeight);
				tempWindowPosUpdated = false;
				tempWindowSizesUpdated = false;
				return true;
			}
			setWindowSize(tempWindowWidth, tempWindowHeight);
			tempWindowSizesUpdated = false;
			return true;
		}
	}
	return false;
}

#define UWM_EXIT_FROM_THREAD WM_USER
bool isAlive = true;
std::thread graphicsThread;

bool listenForExitAttempts(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		isAlive = false;
		graphicsThread.join();
		PostQuitMessage(EXIT_SUCCESS);
		return true;
	case UWM_EXIT_FROM_THREAD:
		graphicsThread.join();
		PostQuitMessage(EXIT_SUCCESS);
		return true;
	}
	return false;
}

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define POST_THREAD_EXIT if (!PostMessage(hWnd, UWM_EXIT_FROM_THREAD, 0, 0)) { debuglogger::out << debuglogger::error << "failed to post UWM_EXIT_FROM_THREAD message to window queue" << debuglogger::endl; }
void graphicsLoop();

#ifdef UNICODE
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* lpCmdLine, int nCmdShow) {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
#endif
	debuglogger::out << "program started" << debuglogger::endl;

	debuglogger::out << "setting up window..." << debuglogger::endl;
	WNDCLASS windowClass = { };																																			// Create WNDCLASS struct for later window creation.
	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = TEXT(WINDOW_CLASS_NAME);
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

	if (!RegisterClass(&windowClass)) {
		debuglogger::out << debuglogger::error << "failed to register window class" << debuglogger::endl;
		return EXIT_FAILURE;
	}

	hWnd = CreateWindow(windowClass.lpszClassName, TEXT(WINDOW_TITLE), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);
	if (hWnd == nullptr) {																																				// Check if creation of window was successful.
		debuglogger::out << debuglogger::error << "couldn't create window" << debuglogger::endl;
		return EXIT_FAILURE;
	}

	debuglogger::out << "showing window..." << debuglogger::endl;
	ShowWindow(hWnd, nCmdShow);

	RECT clientRect;
	if (!GetClientRect(hWnd, &clientRect)) {
		debuglogger::out << debuglogger::error << "failed to get client size" << debuglogger::endl;
		return EXIT_FAILURE;
	}
	POINT clientPos = { 0, 0 };
	if (!ClientToScreen(hWnd, &clientPos)) {
		debuglogger::out << debuglogger::error << "failed to get client pos in screen coords" << debuglogger::endl;
		return EXIT_FAILURE;
	}
	setWindow(clientPos.x, clientPos.y, clientRect.right, clientRect.bottom);

	debuglogger::out << "starting graphics thread..." << debuglogger::endl;
	graphicsThread = std::thread(graphicsLoop);

	debuglogger::out << "running message loop..." << debuglogger::endl;
	MSG msg = { };
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (graphicsThread.joinable()) { graphicsThread.join(); }																											// Join the graphicsThread if the user hasn't done it yet.
	debuglogger::out << "terminating program..." << debuglogger::endl;
	return msg.wParam;																																					// As per documentation, the system return code and the PostQuitMessage return code have to be the same.
}
