#define _CRT_SECURE_NO_WARNINGS																						// Disables warnings for functions that are deemed insecure by the standard. It's only for _itoa, which I am using safely, so I can turn of the warnings.

#include "debugOutput.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdint>
#include <stdlib.h>																									// This is included because we need access to _itoa.

DebugOutput& DebugOutput::operator<<(const char* input) {
#ifndef _DEBUG
	return *this;
#endif
	OutputDebugStringA(input);
	return *this;
}

DebugOutput& DebugOutput::operator<<(char* input) {
#ifndef _DEBUG
	return *this;
#endif
	OutputDebugStringA(input);
	return *this;
}

DebugOutput& DebugOutput::operator<<(char input) {
#ifndef _DEBUG
	return *this;
#endif
	char buffer[] = { input, '\0' };																				// List initialization of basic types is apparently just as efficient as doing it per hand, contrary to custom types and such. Use equal sign because that makes it compatible with C.
	OutputDebugStringA(buffer);
	return *this;
}


DebugOutput& DebugOutput::operator<<(int32_t input) {
#ifndef _DEBUG
	return *this;
#endif
	char buffer[12];
	_itoa(input, buffer, 10);
	OutputDebugStringA(buffer);
	return *this;
}

DebugOutput& DebugOutput::operator<<(uint32_t input) {
#ifndef _DEBUG
	return *this;
#endif
	char buffer[11];
	_itoa(input, buffer, 10);
	OutputDebugStringA(buffer);
	return *this;
}