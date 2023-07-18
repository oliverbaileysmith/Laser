#ifndef BOUNDS_CL
#define BOUNDS_CL

#include "Ray.cl"

typedef struct Bounds
{
	float3 pMin;
	float3 pMax;
} Bounds;

bool intersectBounds(Ray* ray, Bounds* bounds)
{
    float t0 = 0;
    float t1 = INFINITY;

    // Check each axis-aligned slab in bounds
    for (int i = 0; i < 3; i++) {
        // Precalculate inverse direction
        float invRayDir = 1 / ray->dir[i];
        float tNear = (bounds->pMin[i] - ray->orig[i]) * invRayDir;
        float tFar  = (bounds->pMax[i] - ray->orig[i]) * invRayDir;

        // Reorder tNear and tFar if necessary
        if (tNear > tFar)
        {
            float t = tNear;
            tNear = tFar;
            tFar = t;
        }

        // Update t0 and t1
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar  < t1 ? tFar  : t1;

        // Return if interval is degenerate
        if (t0 > t1) return false;
    }
    return true;
}

#endif // BOUNDS_CL