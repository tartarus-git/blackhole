typedef struct Matrix4f { float data[16]; } Matrix4f;

float3 multMatFloat3(Matrix4f mat, float3 vec) {
	float3 result;
	result.x = mat.data[0] * vec.x + mat.data[4] * vec.y + mat.data[8] * vec.z;
	result.y = mat.data[1] * vec.x + mat.data[5] * vec.y + mat.data[9] * vec.z;
	result.z = mat.data[2] * vec.x + mat.data[6] * vec.y + mat.data[10] * vec.z;
	return result;
}

typedef struct Skybox {
	float3 test;
} Skybox;

float abscus(float x) {
	if (x < 0) { x = -x; }
	return x;
}

uint3 skyboxSample(Skybox skybox, float3 normVec) {
	uint3 result;
	if (abscus(normVec.x) > abscus(normVec.y) && abscus(normVec.x) > abscus(normVec.z)) {
		result.x = 255;
		result.y = 0;
		result.z = 0;
		return result;
	}
	if (abscus(normVec.y) > abscus(normVec.x) && abscus(normVec.y) > abscus(normVec.z)) {
		result.y = 255;
		result.x = 0;
		result.z = 0;
		return result;
	}
	if (abscus(normVec.z) > abscus(normVec.y) && abscus(normVec.z) > abscus(normVec.x)) {
		result.z = 255;
		result.y = 0;
		result.x = 0;
		return result;
	}
	return result;
}

typedef struct Camera {
	float3 pos;
	float3 rayOrigin;
	float nearPlane;
} Camera;

typedef struct Blackhole {
	float3 pos;
	float blackRadius;          // TODO: I'm pretty sure OpenCL GPU's always use 32-bit floats, check that. Make sure you're using 32-bit even on 64-bit mode on host floats. That isn't really super easy to do, and also not super necessary because floats are almost always 32-bit, regardless of 64-bit or 32-bit OS. At least on windows. You can typedef for future correctness though I guess, that would be a good idea.
	float influenceRadius;
} Blackhole;

__kernel void raytracer(__write_only image2d_t outputFrame, unsigned int windowWidth, unsigned int windowHeight, 
							Camera camera, Matrix4f cameraRot, Skybox skybox, Blackhole blackhole) {
	int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

	// NOTE: Z coords go out of the screen towards the viewer.
	float3 pixelPos = (float3)(coords.x - (int)windowWidth / 2, -(coords.y - (int)windowHeight / 2), camera.nearPlane);
	camera.rayOrigin.x = 0;
	camera.rayOrigin.y = 0;
	float3 ray = pixelPos - camera.rayOrigin;
	ray = normalize(ray);
	ray = multMatFloat3(cameraRot, ray);
	camera.rayOrigin = multMatFloat3(cameraRot, ray);
	camera.rayOrigin += camera.pos;

	// Intersection test with the blackhole black body.
	// TODO: If you get a tangent vector between the ray origin and the edge of the blackhole black body, then you don't have to do this complicated quadratic stuff, you can just run a dot product between the ray and the vector between ray origin and blackhole middle and compare it to aforementioned vector. You'll basically be making use out of the rotation of the viewport and saving processing power I think.
	// The above will also be super useful when calculating the light shell around the black body.

	// For now, we're doing it like this:
	//float3 sum = camera.rayOrigin - blackhole.pos;
	//float3 p = -sum / ray;
	//float3 q2 = (camera.rayOrigin - blackhole.pos);
	//float q = (dot(q2, q2) - blackhole.blackRadius * blackhole.blackRadius) / dot(ray, ray);
	// TODO: The following is plagarized, you should figure it out for yourself. In order to do that you have to establish basic algebraic rules for vectors, shouldn't be hard. Then do some basic quadratic stuff.
	//if (dot(dot(ray, camera.rayOrigin - blackhole.pos), dot(ray, camera.rayOrigin - blackhole.pos)) - dot(camera.rayOrigin - blackhole.pos, camera.rayOrigin - blackhole.pos) + blackhole.blackRadius * blackhole.blackRadius >= 0) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }		// Hit the black hole.


	float3 toBlackhole = blackhole.pos - camera.rayOrigin;
	float3 fromBlackhole = -toBlackhole;
	float targetDot = (dot(fromBlackhole, fromBlackhole) - blackhole.blackRadius * blackhole.blackRadius); 
	if (dot(ray, fromBlackhole) * dot(ray, fromBlackhole) >= targetDot) { write_imageui(outputFrame, coords, (uint4)(0, 0, 0, 255)); return; }


	uint3 color = skyboxSample(skybox, ray);
	write_imageui(outputFrame, coords, (uint4)(color.x, color.y, color.z, 255));
}