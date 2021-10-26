struct Vector3f {
    float x;
    float y;
    float z;
}

struct Camera {
    Vector3f pos;
    Vector3f rot;
}

struct Skybox {
    // TODO: implement.
}

struct Blackhole {
    Vector3f pos;
    float blackRadius;          // TODO: Find out a way to make sure you use 32-bit floats on host as well as device. That might not always be the case.
    float influenceRadius;
}

__kernel void raytracer(__write_only image2d_t outputFrame, unsigned int windowWidth, unsigned int windowHeight, 
                            Camera camera, Skybox skybox, Blackhole blackhole) {
    int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

    write_imageui(outputFrame, coords, (uint4)(255, 255, 0, 0));
}