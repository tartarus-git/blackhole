#pragma once

#include <cstdint>
#include <string>

class DebugOutput {
public:
	DebugOutput& operator<<(const char* input);
	DebugOutput& operator<<(char* input);
	DebugOutput& operator<<(char input);

	DebugOutput& operator<<(std::string& input);

	DebugOutput& operator<<(int32_t input);
	DebugOutput& operator<<(uint32_t input);
};

namespace debuglogger {
	static DebugOutput out;

	static const char* endl = "\n";
	static const char* error = "ERROR: ";
}
