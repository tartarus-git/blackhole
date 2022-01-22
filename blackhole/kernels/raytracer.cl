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

__kernel void raytracer(__write_only image2d_t outputFrame, int windowWidth, int windowHeight, 
							float3 cameraPos, float rayOrigin, Matrix4f cameraRot, 
							Skybox skybox, 
							float3 blackholePos, float blackholeBlackDotProduct, float blackholeInfluenceDotProduct) {
	int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

	// NOTE: Z coords go out of the screen towards the viewer.

	// TODO: No reason to keep dividing the window sizes by two every kernel run. Preprocess on host.
	float3 ray = normalize((float3)(coords.x - windowWidth / 2, windowHeight / 2 - coords.y, 0) - (float3)(0, 0, rayOrigin));
	ray = multiplyMatWithFloat3(cameraRot, ray);

// TODO: Go through the math that is needed for the following part again.
	//if (toBlackhole.x * toBlackhole.x + toBlackhole.y * toBlackhole.y + toBlackhole.z * toBlackhole.z <= blackhole.blackRadius * blackhole.blackRadius) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }		// Needed for when your inside the blackhole because the dot product method that follows doesn't handle being inside the blackhole well at all.
	float3 fromBlackhole = cameraPos - blackholePos;
	float rayBlackholeDot = dot(ray, fromBlackhole);
	if (rayBlackholeDot <= 0 && rayBlackholeDot * rayBlackholeDot >= blackholeBlackDotProduct) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }

	//write_imageui(outputFrame, coords, (uint4)((uint)(ray.x * 100), (uint)(ray.y * 100), (uint)(ray.z * 100), 255));
	write_imageui(outputFrame, coords, (uint4)(skyboxSample(skybox, ray), 255));
}