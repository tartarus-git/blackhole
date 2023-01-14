# blackhole

A simulation of optical effects around a blackhole. Uses raytracing and gravitational simulation to achieve close to realistic results.

Note that I don't know (yet) the complicated math required to make this completely realistic, so my "naive" approach results in some
inaccuracies, like the fact that you can see a second instance of the blackhole behind you when you get close to the blackhole. As far as I know, that shouldn't happen in real life.

# How To Build

It's a Visual Studio Solution. Clone the repo (recursively, it's got submodules) and then open it up in Visual Studio and build/run it.

# How To Use Program

There's a couple of input keys that you need to be aware of to use the program:
```
wasd --> move
mouse --> look
space --> up
ctrl --> down
p --> increase the value for the speed of light that the GPU code uses for the simulation
o --> decrease that value
k --> decrease the value for the amount of steps that the GPU code does with the light ray when simulating
l --> increase the that value
```

# TODO: Add pictures
