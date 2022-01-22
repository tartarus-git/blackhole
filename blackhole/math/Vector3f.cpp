#include "Vector3f.h"

Vector3f Vector3f::rotate(const Vector3f& rot) const noexcept {			// NOTE: This should be constexpr but the trig functions don't allow that for some reason. Can't write our own because the trig functions might be optimized for each platform and writing our own would lose us performance.
	Vector3f result;

	float cosine = cos(rot.y);
	float sine = sin(rot.y);
	result.y = cosine * y - sine * z;
	result.z = sine * y + cosine * z;

	cosine = cos(rot.x);
	sine = sin(rot.x);
	result.x = cosine * x - sine * result.z;
	result.z = sine * x + cosine * result.z;

	cosine = cos(rot.z);
	sine = sin(rot.z);
	float cachedX = result.x;
	result.x = cosine * result.x - sine * result.y;
	result.y = sine * cachedX + cosine * result.y;

	return result;
}
