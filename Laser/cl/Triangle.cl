#include "RenderStats.cl"

struct Triangle
{
	unsigned int v0;
	unsigned int v1;
	unsigned int v2;
	unsigned int MaterialIndex;
};

bool intersectTriangle(struct Ray* ray, float3 v0, float3 v1, float3 v2,
	float* t, float3* n, __global struct RenderStats* renderStats)
{
	//atomic_inc(&(renderStats->n_RayTriangleTests));

	// calculate triangle/plane normal
	float3 v0v1 = v1 - v0;
	float3 v0v2 = v2 - v0;
	float3 normal = cross(v0v1, v0v2);
	normal = normalize(normal);

	// calculate intersection point of ray and the plane in which the triangle lies
	float D = - dot(normal, v0); // distance from origin to plane parallel normal

	float denom = dot(normal, ray->dir);
    if (fabs(denom) < EPSILON) // test for ray parallel to triangle
        return false;

	*t = -(dot(normal, ray->orig) + D) / denom;
	if (*t < 0.0f) // test if hit point is behind camera
		return false;

	float3 p = ray->orig + *t * ray->dir; // intersection point

	// check if intersection point is inside triangle
	float3 C;
	
	float3 edge0 = v1 - v0; // first edge
	float3 v0p = p - v0;
	C = cross(edge0, v0p);
	if (dot(normal, C) < 0.0f)
		return false;

	float3 edge1 = v2 - v1; // second edge
	float3 v1p = p - v1;
	C = cross(edge1, v1p);
	if (dot(normal, C) < 0.0f)
		return false;
	

	float3 edge2 = v0 - v2; // third edge
	float3 v2p = p - v2;
	C = cross(edge2, v2p);
	if (dot(normal, C) < 0.0f)
		return false;
	
	*n = normal;
	//atomic_inc(&(renderStats->n_RayTriangleIsects));
	return true;
}