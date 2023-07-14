#include "RenderStats.cl"

struct Triangle
{
	unsigned int v0;
	unsigned int v1;
	unsigned int v2;
	unsigned int Material;
	unsigned int Transform;
};

bool intersectTriangle(struct Ray* ray, float3 v0, float3 v1, float3 v2,
	float* t, float3* n, float* u, float* v, __global struct RenderStats* renderStats)
{
	// Moller Trumbore from https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/

	float3 v0v1 = v1 - v0;
	float3 v0v2 = v2 - v0;
	float3 h = cross(ray->dir, v0v2);
	float a = dot(v0v1, h);
	if (a > -EPSILON && a < EPSILON) return false; // ray parallel to triangle

	float f = 1 / a;
    float3 s = ray->orig - v0;
    *u = f * dot( s, h );
    if (*u < 0 || *u > 1) return false;

    float3 q = cross( s, v0v1 );
    *v = f * dot( ray->dir, q );
    if (*v < 0 || *u + *v > 1) return false;

    float tt = f * dot( v0v2, q );
    if (tt > EPSILON) *t = min( *t, tt );

	float3 normal = cross(v0v1, v0v2);
	normal = normalize(normal);
	*n = normal;

	return true;
}