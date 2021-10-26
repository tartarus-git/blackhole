__kernel void raytracer(__write_only image2d_t outputFrame, unsigned int windowWidth, unsigned int windowHeight) {
    int x = get_global_id(0);
	if (x >= windowWidth) { return; }
	int2 coords = (int2)(x, get_global_id(1));

    write_imageui(outputFrame, coords, (uint4)(255, 255, 0, 0));
}