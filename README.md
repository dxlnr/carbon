# Rendering Engine


## Path Tracer Overview

1. Initialization
    - Scene : Define the objects, materials, and lights in the scene.
    - Camera: Define the camera's position, orientation, field of view, etc.
2. Ray Generation
    - For each pixel in the image: Generate a primary ray starting from the camera and passing through the pixel.
3. Ray Intersection
    - For each ray:
        - Check for intersection with all objects in the scene.
        - Find the nearest intersection point.
4. Shading
    - For each intersection:
        - If it hits a light source, add the light's contribution to the pixel. Otherwise, generate a new ray in a random direction based on the material properties (reflection, refraction, etc.).
        - Recursively trace the new ray until a maximum depth is reached or until the ray hits a light source.
5. Accumulation
    - For each pixel:
        - Average the colors from all the rays that passed through the pixel.
        - This can be done over multiple iterations to improve image quality.
6. Bias Introductions (for speed or simplicity)
    - Early Termination: Instead of always tracing rays to a maximum depth, you can randomly terminate paths with a certain probability.
    - Approximate Materials: Instead of simulating complex materials, use simpler approximations.
    - Limit Light Bounces: Instead of allowing many bounces, limit rays to 1 or 2 bounces.
    - Use Coarse Samples: Instead of sending many rays per pixel, send just a few and reuse or interpolate results.
7. Post-Processing
    - Apply tonemapping to convert high dynamic range (HDR) values to displayable low dynamic range (LDR) values.
    - Apply denoising techniques to reduce noise, especially if fewer samples per pixel are used.
8. Optimizations
    - Acceleration Structures: Use structures like BVH (Bounding Volume Hierarchies) or grids to speed up ray-object intersection tests.
    - Parallelism: Use multi-threading or GPU acceleration to trace multiple rays simultaneously.
