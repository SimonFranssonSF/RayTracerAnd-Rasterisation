# RayTracerAndRasterisation

Part of course DH2323 at KTH. In this project sdl with gml is used to build a raytracer as well as a rasterizer for a given scene. The position of the camera and the lightsource (omni light) can be moved by using the keyboard. The scene and its illumination are updated after moving the viewpoint/lightsource's position. 

The raytracer: each ray is traced to find the intersection/closest intersection to see what color a point/pixel should be. Illumination of direct light from the lightsource is based on power*max(dot(directionToLightsource, surfaceNormal),0)/4(pi)r where r is the distance to the lightsource. Indirect light is approximated using a constant. All surfaces are reflects the same amount of light. A ray from each. Rays are sent from the surface to the lightsource to see if there is any intersection closer than than the distance to the lightsource (in that case the poin in the surface shouldn't receive any direct light as it is behind/in shadow).

The rasterization: Projects vertices in 3d world to 2d image plane, draws the edges of the triangles, drawing rows/filling triangles, depth buffer to draw only if current depth is smaller than the depth stored in a pixel to be drawed to. depths are interpolated from vertices over triangles. No shadows. Illumination computed for every pixel not just for vertices and then interpolated.

![Alt text](/screenshots/6.png?raw=true)
![Alt text](/screenshots/5.png?raw=true)
![Alt text](/screenshots/3.png?raw=true)
![Alt text](/screenshots/2.png?raw=true)
![Alt text](/screenshots/1.png?raw=true)
