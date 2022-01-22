typedef struct Camera {
	float3 pos;
	float rayOrigin;
} Camera;

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
	if (abs(normVec.x) > abs(normVec.y) && abs(normVec.x) > abs(normVec.z)) {
		result.x = 255;
		result.y = 0;
		result.z = 0;
		return result;
	}
	if (abs(normVec.y) > abs(normVec.x) && abs(normVec.y) > abs(normVec.z)) {
		result.y = 255;
		result.x = 0;
		result.z = 0;
		return result;
	}
	if (abs(normVec.z) > abs(normVec.y) && abs(normVec.z) > abs(normVec.x)) {
		result.z = 255;
		result.y = 0;
		result.x = 0;
		return result;
	}
	return result;
}

typedef struct Blackhole {
	float3 pos;
	float blackDotProduct;
	float influenceDotProduct;
} Blackhole;

__kernel void raytracer(__write_only image2d_t outputFrame, int windowWidth, int windowHeight, 
							Camera camera, Matrix4f cameraRot, Skybox skybox, Blackhole blackhole) {
	int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

	// NOTE: Z coords go out of the screen towards the viewer.

	float3 ray = normalize((float3)(coords.x - windowWidth / 2, windowHeight / 2 - coords.y, 0) - (float3)(0, 0, camera.rayOrigin));
	ray = multiplyMatWithFloat3(cameraRot, ray);

	float3 toBlackhole = blackhole.pos - camera.rayOrigin;
	if (toBlackhole.x * toBlackhole.x + toBlackhole.y * toBlackhole.y + toBlackhole.z * toBlackhole.z <= blackhole.blackRadius * blackhole.blackRadius) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }		// Needed for when your inside the blackhole because the dot product method that follows doesn't handle being inside the blackhole well at all.
	float3 fromBlackhole = -toBlackhole;
	float targetDot = (dot(fromBlackhole, fromBlackhole) - blackhole.blackRadius * blackhole.blackRadius); 
	float rayBlackholeDot = dot(ray, fromBlackhole);
	if (rayBlackholeDot <= 0 && rayBlackholeDot * rayBlackholeDot >= blackhole.blackDotProduct) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }

	write_imageui(outputFrame, coords, (uint4)(skyboxSample(skybox, ray), 255));
}