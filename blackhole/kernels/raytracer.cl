typedef struct Vector3f {
    float x;
    float y;
    float z;
} Vector3f;

// TODO: See if you can just use the built in type vec3 or something and stop making every operation from scratch.
Vector3f vec3Sub(Vector3f a, Vector3f b) {
    Vector3f result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

float vec3Magnitude(Vector3f a) {
    float width = a.x * a.x;            // TODO: Fix var names. Don't use vars at all.
    float height = a.y * a.y;
    float depth = a.z * a.z;
    return sqrt(width + height + depth);
}

Vector3f vec3Normalize(Vector3f a) {
    float mag = vec3Magnitude(a);
    Vector3f result;
    result.x = a.x / mag;
    result.y = a.y / mag;           // TODO: No way to avoid division here?
    result.z = a.z / mag;
    return result;
}

typedef struct Camera {
    Vector3f pos;
    Vector3f rot;

    float FOV;              // TODO: Find a way to not pass this into GPU because it is unnecessary information. Make another class that camera maybe inherits from, or doesn't, something like that.
    float nearPlane;
} Camera;

typedef struct Skybox {
    Vector3f test;
} Skybox;

float abscus(float x) {
    if (x < 0) { x = -x; }
    return x;
}

uint3 skyboxSample(Skybox skybox, Vector3f normVec) {
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
    Vector3f pos;
    float blackRadius;          // TODO: Find out a way to make sure you use 32-bit floats on host as well as device. That might not always be the case.
    float influenceRadius;
} Blackhole;

__kernel void raytracer(__write_only image2d_t outputFrame, unsigned int windowWidth, unsigned int windowHeight, 
                            Camera camera, Vector3f rayOrigin, Skybox skybox, Blackhole blackhole) {
    int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

    // NOTE: Z coords go out of the screen towards the viewer.
    Vector3f coords3D = { coords.x - rayOrigin.x, coords.y - rayOrigin.y, -camera.nearPlane };
    rayOrigin.x = 0;
    rayOrigin.y = 0;            // TODO: This is necessary because the host is populating the data wrong, fix that.
    Vector3f ray = vec3Sub(coords3D, rayOrigin);
    ray = vec3Normalize(ray);

    // Rotate ray and rayOrigin based on camera look direction.
    float cameraCosX = cos(camera.rot.x);
    float cameraSinX = sin(camera.rot.x);           // TODO: You can offload these calculations to the host without problem.
                                                    // TODO: You should defo use matrices for this, because GPU might be optimized for that.

    float cameraCosY = cos(camera.rot.y);
    float cameraSinY = sin(camera.rot.y);

    ray.x += rayOrigin.x;
    ray.y += rayOrigin.y;
    ray.z += rayOrigin.z;

    Vector3f rayCopy = ray;
    Vector3f rayOriginCopy = rayOrigin;

    rayOrigin.x = cameraCosX * rayOriginCopy.x - cameraSinX * rayOriginCopy.z;
    rayOrigin.z = cameraSinX * rayOriginCopy.x + cameraCosX * rayOriginCopy.z;

    ray.x = cameraCosX * rayCopy.x - cameraSinX * rayCopy.z;
    ray.z = cameraSinX * rayCopy.x + cameraCosX * rayCopy.z;

    rayOriginCopy = rayOrigin;
    rayCopy = ray;

    rayOrigin.z = cameraCosY * rayOriginCopy.z - cameraSinY * rayOriginCopy.y;
    rayOrigin.y = cameraSinY * rayOriginCopy.z + cameraCosY * rayOriginCopy.y;

    ray.z = cameraCosY * rayCopy.z - cameraSinY * rayCopy.y;
    ray.y = cameraSinY * rayCopy.z + cameraCosY * rayCopy.y;

    ray.x -= rayOrigin.x;
    ray.y -= rayOrigin.y;
    ray.z -= rayOrigin.z;

    uint3 color = skyboxSample(skybox, ray);
    write_imageui(outputFrame, coords, (uint4)(color.x, color.y, color.z, 255));
}