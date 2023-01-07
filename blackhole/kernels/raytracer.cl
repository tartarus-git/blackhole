typedef struct Matrix4f { float data[16]; } Matrix4f;

float3 multiplyMatWithFloat3(Matrix4f mat, float3 vec) {
	float3 result;
	result.x = mat.data[0] * vec.x + mat.data[1] * vec.y + mat.data[2] * vec.z;
	result.y = mat.data[4] * vec.x + mat.data[5] * vec.y + mat.data[6] * vec.z;
	result.z = mat.data[8] * vec.x + mat.data[9] * vec.y + mat.data[10] * vec.z;
	return result;
}

float4 multiplyMatWithFloat4(Matrix4f mat, float4 vec) {
	float4 result;
	result.x = mat.data[0] * vec.x + mat.data[1] * vec.y + mat.data[2] * vec.z + mat.data[3] * vec.w;
	result.y = mat.data[4] * vec.x + mat.data[5] * vec.y + mat.data[6] * vec.z + mat.data[7] * vec.w;
	result.z = mat.data[8] * vec.x + mat.data[9] * vec.y + mat.data[10] * vec.z + mat.data[11] * vec.w;
	result.w = mat.data[12] * vec.x + mat.data[13] * vec.y + mat.data[14] * vec.z + mat.data[15] * vec.w;
	return result;
}

typedef struct Skybox {
	float3 test;
} Skybox;

uint3 skyboxSample(Skybox skybox, float3 normVec) {
	uint3 result;
	if (fabs(normVec.x) > fabs(normVec.y) && fabs(normVec.x) > fabs(normVec.z)) {
		result.x = 255;
		result.y = 0;
		result.z = 0;
		return result;
	}
	if (fabs(normVec.y) > fabs(normVec.x) && fabs(normVec.y) > fabs(normVec.z)) {
		result.y = 255;
		result.x = 0;
		result.z = 0;
		return result;
	}
	if (fabs(normVec.z) > fabs(normVec.y) && fabs(normVec.z) > fabs(normVec.x)) {
		result.z = 255;
		result.y = 0;
		result.x = 0;
		return result;
	}
	return result;
}

// TODO: Does inline actually do anything in OpenCL C? Is it like C++ and normal C where it doesn't actually mean that much regarding inlining?

inline void colorDisc(__write_only image2d_t outputFrame, int2 coords) {
	write_imageui(outputFrame, coords, (uint4)(255));
}

inline void colorBlackhole(__write_only image2d_t outputFrame, int2 coords) {
	write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255));
}

inline void colorSky(__write_only image2d_t outputFrame, int2 coords, Skybox skybox, float3 ray) {
	write_imageui(outputFrame, coords, (uint4)(skyboxSample(skybox, ray), 255));
}


inline float intersectLineHorizontalCircle(float3 origin, float3 ray, float3 circlePos, float circleRadius) {
	// TODO: This should actually be a donut type thing instead of a circle. Return -2 or something if ray hits in the donut hole and then you can avoid blackhole calculations in the calling code because of logic.

	float yDiff = origin.y - circlePos.y;
	yDiff = -yDiff;					// TODO: Without this line, the sim is wrong. Why is it messed up in that specific way that it is though? Seems really strange just because of this one line.

	// TODO: The weirdness is actually also present in the normal version with this line in, but I don't think it has anything to do with this part of the code.
	// I think the fact that we are using Kleinschrittverfahren for Gravity instead of smooth version with some fancy integral is causing issues.
	// Close to the blackhole, some rays hit the blackhole instead of the disc because the gravity curve isn't smooth but blocky so to speak.
	// Decreasing light speed, increasing step count, increasing mass of blackhole, so that the simulation stays relatively same while the "resolution" of the gravity goes up, doing that mitigates the issues, which supports my theory.
	// That causes other optical effects though. Before solving this issue, we should take care of the crazy distortion that the player experiences when objects go into corners. That shouldn't happen I think.


	if (ray.y == 0 && yDiff == 0) { return 0; }
	if (ray.y == 0) { return -1; }
	float fract = yDiff / ray.y;
	if (fract < 0) { return -1; }
	ray *= fract;

	float3 planeCoords = origin + ray;
	if (length(planeCoords - circlePos) > circleRadius) { return -1; }
	return fract;

}

inline float intersectLineSphere(float3 origin, float3 ray, float3 spherePos, float sphereRadius) {

	float otherTerm = -(dot((origin - spherePos), ray));
	float determinant = dot(dot((origin - spherePos), ray), dot((origin - spherePos), ray)) - dot(ray, ray) * (dot(origin - spherePos, origin - spherePos) - sphereRadius * sphereRadius);

	if (determinant > 0) {
		determinant = sqrt(determinant);
		float d0 = (otherTerm + determinant) / dot(ray, ray);
		float d1 = (otherTerm - determinant) / dot(ray, ray);
		if (d0 <= d1) {
			if (d0 < 0) {
				if (d1 < 0) {
					return -1;
				}
				return d1;
			}
			return d0;
		} else {
			if (d1 < 0) {
				if (d0 < 0) {
					return -1;
				}
				return d0;
			}
			return d1;
		}
	} else if (determinant == 0) {
		float distance = otherTerm / dot(ray, ray);
		if (distance < 0) { return -1; }
		return distance;
	} else {
		return -1;
	}

}


__kernel void raytracer(__write_only image2d_t outputFrame, int windowWidth, int windowHeight, float halfWindowWidth, float halfWindowHeight, 
							float3 cameraPos, float rayOrigin, Matrix4f cameraRot, 
							Skybox skybox, 
							float3 blackholePos, float blackholeMass, float blackholeBlackRadius, float blackholeInfluenceRadius) {
	int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

	// NOTE: Z coords go out of the screen towards the viewer.

	float3 ray = multiplyMatWithFloat3(cameraRot, normalize((float3)(coords.x - halfWindowWidth, halfWindowHeight - coords.y, -rayOrigin)));

	float3 rayPosition = cameraPos;

float blackholeDistance;
float discDistance;

		
		ray *= 1;			// multiplier is light speed in this case.
		for (uint i = 0; i < 30; i++) {

			blackholeDistance = intersectLineSphere(rayPosition, ray, blackholePos, blackholeBlackRadius);
			discDistance = intersectLineHorizontalCircle(rayPosition, ray, blackholePos, blackholeBlackRadius + 20);
			if (blackholeDistance != -1 && blackholeDistance <= length(ray)) {
				if (discDistance != -1 && discDistance <= length(ray)) {
					if (discDistance <= blackholeDistance) {
						goto templabel;
						colorDisc(outputFrame, coords); return;
					}
					colorBlackhole(outputFrame, coords); return;
				}
				colorBlackhole(outputFrame, coords); return;
			}
			if (discDistance != -1 && discDistance <= length(ray)) {
				goto templabel;
				colorDisc(outputFrame, coords); return;
			}

templabel:
			rayPosition += ray;
			// G = 1 for now.
			float accel = 1 * blackholeMass / dot((blackholePos - rayPosition), (blackholePos - rayPosition));

			float3 toVec = blackholePos - rayPosition;
			toVec = normalize(toVec);
			ray += toVec * accel;




		}

			blackholeDistance = intersectLineSphere(rayPosition, ray, blackholePos, blackholeBlackRadius);
			if (blackholeDistance == -1) {
				discDistance = intersectLineHorizontalCircle(rayPosition, ray, blackholePos, blackholeBlackRadius + 20);
				if (true) {
					colorSky(outputFrame, coords, skybox, normalize(ray)); return;
				}
				colorDisc(outputFrame, coords); return;
			}
			discDistance = intersectLineHorizontalCircle(rayPosition, ray, blackholePos, blackholeBlackRadius + 20);
			if (discDistance == -1) {
				colorBlackhole(outputFrame, coords); return;
			}
			if (false) {
				colorDisc(outputFrame, coords); return;
			}
			colorBlackhole(outputFrame, coords); return;



	write_imageui(outputFrame, coords, (uint4)(255, 0, 0, 255));				// Just for debugging, probs remove later.
}