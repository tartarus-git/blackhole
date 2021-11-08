typedef struct Matrix4f { float data[16]; } Matrix4f;

float3 multMatFloat3(Matrix4f mat, float3 vec) {
    float3 result;
    result.x = mat.data[0] * vec.x + mat.data[1] * vec.y + mat.data[2] * vec.z;
    result.y = mat.data[4] * vec.x + mat.data[5] * vec.y + mat.data[6] * vec.z;
    result.z = mat.data[8] * vec.x + mat.data[9] * vec.y + mat.data[10] * vec.z;
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

typedef struct Blackhole {
    float3 pos;
    float blackRadius;          // TODO: Find out a way to make sure you use 32-bit floats on host as well as device. That might not always be the case.
    float influenceRadius;
} Blackhole;

__kernel void raytracer(__write_only image2d_t outputFrame, unsigned int windowWidth, unsigned int windowHeight, 
                            float3 cameraPos, Matrix4f cameraRot, float nearPlane, float3 rayOrigin, Skybox skybox, Blackhole blackhole) {
    int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

    // NOTE: Z coords go out of the screen towards the viewer.
    float3 ray = (float3)(coords.x, coords.y, nearPlane) - rayOrigin;
    ray = normalize(ray);
    ray = multMatFloat3(cameraRot, ray);

    uint3 color = skyboxSample(skybox, ray);
    write_imageui(outputFrame, coords, (uint4)(color.x, color.y, color.z, 255));
}