#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

class FrameTimer {
public:
	std::chrono::nanoseconds max_frame_duration_in_nanoseconds;
	std::chrono::time_point<std::chrono::high_resolution_clock> frame_start;

	float travel_distance_multiplier = 1;			// NOTE: We set this so that the first iteration of the main loop can use this, even before having been able to calculate it with wait_if_needed().
													// NOTE: From what I could fine, reading undefined variables is UB. It could have been that it would simply give you an some value, but I think it's actually proper UB.
													// I don't exactly know why though, there isn't much online to clear this issue up.
													// TODO: Research more and find a more concrete solution. BTW, trap representations aren't a great explanation because C++ doesn't use the register keyword and we're able to assume everything is in RAM AFAIK. And in RAM,
													// unsigned integers are guaranteed to not have trap representations because they're always two's compliment, so the questions of whether their uninitialized versions are UB still remains.
													// For floats, trap representations are a good explanation, but I still want to know exactly where UB ends and seemingly random numbers begin.

	constexpr FrameTimer(uint16_t max_FPS) noexcept {
		constexpr uint64_t nanoseconds_per_second = (uint64_t)1000 * (uint64_t)1000 * (uint64_t)1000;
		// NOTE: std::chrono::nanoseconds contains a signed representation of nanoseconds, just so you know.
		max_frame_duration_in_nanoseconds = (std::chrono::nanoseconds)(nanoseconds_per_second / max_FPS);
	}

	void mark_frame_start() noexcept { frame_start = std::chrono::high_resolution_clock::now(); }

	void wait_if_needed() noexcept {
		std::chrono::nanoseconds frame_duration = (std::chrono::nanoseconds)(std::chrono::high_resolution_clock::now() - frame_start);
		// TODO: invert if statement for efficiency
		if (frame_duration < max_frame_duration_in_nanoseconds) {
			std::this_thread::sleep_for(max_frame_duration_in_nanoseconds - frame_duration);
			travel_distance_multiplier = 1;
			return;
		}
		// IMPORTANT: The deal with floats is this: Any conversion involving a float causes UB if one type cannot fit into the other.
		// For integer to float, if the integer value is too large, the conversion is UB.
		//		--> In practice, this doesn't matter too much because a) the maximum values of floats and doubles are way bigger than the biggest integers, so you're fine.
		//				and b) if you're using IEEE (or something else that supports infinity), you've got infinity, so every single integer will always fit anyway.
		//		--> All this means that the below usage of float cast is totally and utterly ok in almost every single situation (unless we're on a platform where float is a really narrow type and doesn't support infinity, which almost definitely won't happen).
		// For float to integer, the fractional part will be truncated and the integer part must fit within the bounds of the target type. If that's not the case, then UB.
		// For float to double or other floating point types, values that are in between two target values will be rounded (it's implementation defined how they will be rounded). Otherwise, if source value is too large, then UB, but that doesn't happen most of the time, see above.
		travel_distance_multiplier = (float)frame_duration.count() / max_frame_duration_in_nanoseconds.count();
	}
};
