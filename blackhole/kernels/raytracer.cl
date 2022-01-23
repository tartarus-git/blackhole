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

inline void calculateRayBlackholeCollisionDist(float3 origin, float3 ray, float3 fromBlackholeVec, float placeholder) {
	float rayBlackholeDot = dot(ray, fromBlackholeVev);
	if (rayBlackholeDot <= 0 && rayBlackholeDot * rayBlackholeDot >= placeholder) {}
}

inline void calculateRayBlackholeDiscCollisionDist(float3 origin, float3 ray, float3 blackholePos, float3 blackholeDiscRadius) {

}

inline void simulateLight(float3 origin, float3 ray, float rayVel, float3 fromBlackholeVec, float3 blackholePos, float blackholeBlackRadius, float blackholeMass, float blackholeDiscRadius, uint stepCount, Skybox skybox, __write_only image2d_t outputFrame, int2 coords) {
	ray *= vel;

	float fromBlackholeVecDistSquared = dot(fromBlackholeVec, fromBlackholeVec);					// TODO: Does normalize use sqrt behind the scenes. If it doesn't, then the way I'm calculating inside of the for loop is probably suboptimal.
	for (uint i = 0; i < stepCount; i++) {
			/*if ((blackholePos.y - origin.y < 0 && blackholePos.y - (origin + ray).y > 0) || (blackholePos.y - origin.y > 0 && blackholePos.y - (origin + ray).y < 0)) {
				float diffheight = blackholePos.y - origin.y;
				float3 tempray = ray;
				diffheight = diffheight / tempray.y;
				tempray *= diffheight;
				float3 diffVec = origin + tempray - blackholePos;
				if (diffVec.x * diffVec.x + diffVec.z * diffVec.z <= 15 * 15) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }
				if (diffVec.x * diffVec.x + diffVec.z * diffVec.z < RAD * RAD) {
					write_imageui(outputFrame, coords, (uint4)(255, 255, 255, 255)); return;
				}
			}*/

		float distToBlackhole = calculateRayBlackholeCollisionDist(origin, ray, blackholePos, blackholeBlackRadius);
		float distToBlackholeDisc = calculateRayBlackholeDiscCollisionDist(origin, ray, blackholePos, blackholeDiscRadius);
		if (distToBlackhole < distToBlackholeDisc) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); } else { write_imageui(outputFrame, coords, (uint4)(255, 255, 255, 255)); }

		ray -= blackholeMass / fromBlackholeVecDistSquared / sqrt(fromBlackholeVecDistSquared) * fromBlackHoleVec;			// TODO: Look into the different types of sqrt in opencl.
		origin += ray;
		fromBlackholeVec += ray;
		fromBlackholeVecDistSquared = dot(fromBlackholeVec, fromBlackholeVec);
	}

	// TODO: This is the place for the final collision tests on blackhole and disc.

	//write_imageui(outputFrame, coords, (uint4)((uint)(fabs(ray.x) * 255), (uint)(fabs(ray.y) * 255), (uint)(fabs(ray.z) * 255), 255));
	write_imageui(outputFrame, coords, (uint4)(skyboxSample(skybox, ray), 255));
}

__kernel void raytracer(__write_only image2d_t outputFrame, int windowWidth, int windowHeight, float halfWindowWidth, float halfWindowHeight, 
							float3 cameraPos, float rayOrigin, Matrix4f cameraRot, 
							Skybox skybox, 
							float3 blackholePos, float3 fromBlackholeVec, float blackHoleAttraction, float blackholeBlackRadius, float blackholeInfluenceRadius, float blackholeBlackSquaredDotProduct, float blackholeInfluenceSquaredDotProduct) {
	int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

	// NOTE: Z coords go out of the screen towards the viewer.

	float3 ray = multiplyMatWithFloat3(normalize((float3)(coords.x - halfWindowWidth, halfWindowHeight - coords.y, -rayOrigin)));

	// TODO: Go through the math that is needed for the following part again.

	if (blackholeBlackSquaredDotProduct <= 0) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); }				// TODO: Think about if there is a way to get rid of this branch. Does the math that follows take care of it for us?

	/*if (rayBlackholeDot <= 0) {
		if (rayBlackholeDot * rayBlackholeDot >= blackholeBlackDotProduct) { }//write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }
		if (rayBlackholeDot * rayBlackholeDot >= blackholeInfluenceDotProduct) {
		}
	}*/
	simulateLight(cameraPos, ray, 5, blackholePos, 1000, 50, coords, outputFrame, skybox); return;

	//write_imageui(outputFrame, coords, (uint4)((uint)(fabs(ray.x) * 255), (uint)(fabs(ray.y) * 255), (uint)(fabs(ray.z) * 255), 255));
	write_imageui(outputFrame, coords, (uint4)(skyboxSample(skybox, ray), 255));
}