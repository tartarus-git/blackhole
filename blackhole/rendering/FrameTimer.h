#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

class FrameTimer {
public:
	std::chrono::nanoseconds max_frame_duration_in_nanoseconds;
	std::chrono::time_point<std::chrono::high_resolution_clock> frame_start;

	constexpr FrameTimer(uint16_t max_FPS) noexcept {
		constexpr uint64_t nanoseconds_per_second = (uint64_t)1000 * (uint64_t)1000 * (uint64_t)1000;
		// NOTE: std::chrono::nanoseconds contains a signed representation of nanoseconds, just so you know.
		max_frame_duration_in_nanoseconds = (std::chrono::nanoseconds)(nanoseconds_per_second / max_FPS);
	}

	void mark_frame_start() noexcept { frame_start = std::chrono::high_resolution_clock::now(); }

	void wait_if_needed() const noexcept {
		std::chrono::nanoseconds frame_duration = (std::chrono::nanoseconds)(std::chrono::high_resolution_clock::now() - frame_start);
		if (frame_duration < max_frame_duration_in_nanoseconds) {
			std::this_thread::sleep_for(max_frame_duration_in_nanoseconds - frame_duration);
		}
	}
};
