struct Bounds
{
	float3 pMin;
	float3 pMax;
};

bool intersectBounds(struct Ray* ray, struct Bounds* bounds, float* t0Hit, float* t1Hit)
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

    // Update t0Hit and t1Hit parameters
    if (t0Hit) *t0Hit = t0;
    if (t1Hit) *t1Hit = t1;
    return true;
}