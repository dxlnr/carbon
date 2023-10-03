# Rendering Engine

```bash
# Build with scons
scons
# Run
./render
# Display help
bash run.sh -h
# Run with args
bash run.sh -w 960 -h 720 -s 1000
```

## Concepts

**Global Illumination** := Given a scene description that specifies 
the location of surfaces in a scene, the location of lights, and the location of a camera. 

**The Rendering Equation**

$L(P \to D_v = L_e (P \to D_v) + \int_\Omega F_s(D_v, D_i) | \cos \theta | L (Y_i \to - D_i) dD_i $

The goal is to calculate the radiance (intensity of light) coming from surface point $P$ in direction $D_v$. 

Using **Monte Carlo Sampling**

$L_o = \frac{1}{N} \sum^N_i \frac{L_i \times BRDF \times \cos (\theta_i) }{p(\omega_i)}$

Where:
- $L_o$ is the outgoing light from the hit point.
- $N$ is the number of sampled rays.
- $L_i$ is the incoming light from the $i^{th}$ sampled direction.
- BRDF is the Bidirectional Reflectance Distribution Function of the surface.
- $\cos (\theta_i)$ is the cosine of the angle between the $i^{th}$ sampled direction and the normal.
- $p(\omega_i)$ is the probability density function (pdf) of the $i^{th}$ sampled direction. For cosine-weighted hemisphere sampling, $p(\omega_i) = \cos (\theta_i) / \pi$

## Path Tracer Overview

(Pre) **Initialization**

- Scene : Define the objects, materials, and lights in the scene.
- Camera: Define the camera's position, orientation, field of view, etc.

```python
for each pixel(i,j) do
  Vec3 C = 0
  for (k=0; k < samplesPerPixel; k++) do
    Create random ray in pixel:
      Choose random point on len P_lens
      Choose random point on image plane P_images
      D = normalize(P_image - P_lens)
      Ray ray = Ray(P_lens, D)
    castRay(ray, isect)
    if ray hits something then
      C += radiance(ray, iscect, 0)
    else
      C += backgroundColor(D) 
    end if
  end for 
  image(i,j) = C/samplesPerPixel
end for
```

(Algo) **Bias Introductions** (for speed or simplicity)

- Early Termination: Instead of always tracing rays to a maximum depth, you can randomly terminate paths with a certain probability.
- Approximate Materials: Instead of simulating complex materials, use simpler approximations.
- Limit Light Bounces: Instead of allowing many bounces, limit rays to 1 or 2 bounces.
- Use Coarse Samples: Instead of sending many rays per pixel, send just a few and reuse or interpolate results.

(Post) **Post-Processing**
- Apply tonemapping to convert high dynamic range (HDR) values to displayable low dynamic range (LDR) values.
- Apply denoising techniques to reduce noise, especially if fewer samples per pixel are used.

(Post) **Optimizations**
- Acceleration Structures: Use structures like BVH (Bounding Volume Hierarchies) or grids to speed up ray-object intersection tests.
- Parallelism: Use multi-threading or GPU acceleration to trace multiple rays simultaneously.


## Notes
- [Scratchapixel 3.0](https://www.scratchapixel.com/): Introduces to computer graphics
- [Ray Tracing in One Weekend: The Book Series](https://raytracing.github.io/)
- [Ray Tracing](https://en.wikipedia.org/wiki/Ray_tracing_(graphics)): Overview
- [smallpt](https://www.kevinbeason.com/smallpt/?source=post_page-----4d9d3ce5fea4--------------------------------): Global Illumination in 99 lines of C++
