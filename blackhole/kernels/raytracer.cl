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

uint3 skyboxSample(Skybox skybox, Vector3f normVec) {
    uint3 result;
    result.x = abs((int)(normVec.x * 255));         // TODO: Figure out why the abs is necessary here. Should integer overflow take care of this for us. Just undo and test.
    result.y = abs((int)(normVec.y * 255));
    result.z = abs((int)(normVec.z * 255));          // TODO: The probabilities of this are a bit off. You should use round before casting.
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
    float cameraSinX = sin(camera.rot.x);

    rayOrigin.x = cameraCosX * rayOrigin.x - cameraSinX * rayOrigin.y;
    rayOrigin.y = cameraSinX * rayOrigin.x + cameraCosX * rayOrigin.y;

    ray.x = cameraCosX * ray.x - cameraSinX * ray.y;
    ray.y = cameraSinX * ray.x + cameraCosX * ray.y;

    uint3 color = skyboxSample(skybox, ray);
    write_imageui(outputFrame, coords, (uint4)(color.x, color.y, color.z, 255));
}