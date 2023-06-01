struct Ray
{
	float3 orig;
	float3 dir;
};

struct Triangle
{
	float3 v0;
	float3 v1;
	float3 v2;
};

struct TriangleIndices
{
	unsigned int v0;
	unsigned int v1;
	unsigned int v2;
};

struct RenderStats
{
	unsigned int n_PrimaryRays;
	unsigned int n_RayTriangleTests;
	unsigned int n_RayTriangleIsects;
	float renderTime;
};

bool intersectTriangle(struct Ray* ray, struct Triangle* triangle, float* t, float3* norm)
{
	// calculate triangle/plane normal (this can be precalculated or included in geometry in future)
	float3 v0v1 = triangle->v1 - triangle->v0;
	float3 v0v2 = triangle->v2 - triangle->v0;
	*norm = cross(v0v1, v0v2);

	// calculate intersection point of ray and the plane in which the triangle lies
	float D = - dot(*norm, triangle->v0); // D is the distance from the origin to the plane, parallel to the plane's normal
	float normDotRayDir = dot(*norm, ray->dir);
    if (fabs(normDotRayDir) < 0.00001f) // test for ray parallel to triangle before continuing, preventing division by 0 when calculating t
        return false;

	*t = - (dot(*norm, ray->orig) + D) / normDotRayDir;
	if (*t < 0.0f) // test if hit point is behind or "inside" camera
		return false;

	float3 p = ray->orig + *t * ray->dir;

	// check if intersection point is inside triangle
	float3 C;
	
	float3 edge0 = triangle->v1 - triangle->v0; // first edge
	float3 v0p = p - triangle->v0;
	C = cross(edge0, v0p);
	if (dot(*norm, C) < 0.0f)
		return false;

	float3 edge1 = triangle->v2 - triangle->v1; // second edge
	float3 v1p = p - triangle->v1;
	C = cross(edge1, v1p);
	if (dot(*norm, C) < 0.0f)
		return false;

	float3 edge2 = triangle->v0 - triangle->v2; // third edge
	float3 v2p = p - triangle->v2;
	C = cross(edge2, v2p);
	if (dot(*norm, C) < 0.0f)
		return false;

	return true;
}

__kernel void Laser(__global float3* output,
	int imageWidth, int imageHeight, float aspectRatio,
	float viewportWidth, float viewportHeight, float focalLength,
	float3 cameraOrigin, float3 upperLeftCorner,
	__global struct Triangle* triangle,
	__global struct RenderStats* stats,
	unsigned int nVertices, __global float3* vertices,
	unsigned int nTriangles, __global struct TriangleIndices* triangles)
{
	// calculate pixel coordinates
	const int workItemID = get_global_id(0);
	int x = workItemID % imageWidth;
	int y = workItemID / imageWidth;
	float fx = (float)x / (float)(imageWidth - 1);
	float fy = (float)y / (float)(imageHeight - 1);

	// generate ray
	atomic_inc(&(stats->n_PrimaryRays));
	struct Ray ray;
	ray.orig = cameraOrigin;
	ray.dir = upperLeftCorner;
	ray.dir.x += fx * viewportWidth;
	ray.dir.y -= fy * viewportHeight;
	ray.dir -= cameraOrigin;
	ray.dir = normalize(ray.dir);

	float t = 0.0f;
	float3 norm = (float3)(0.0f,0.0f,0.0f);
	struct Triangle tri = *triangle;

	atomic_inc(&(stats->n_RayTriangleTests));
	if (intersectTriangle(&ray, &tri, &t, &norm))
	{
		atomic_inc(&(stats->n_RayTriangleIsects));
		output[workItemID] = (float3)(1.0f, 0.0f, 0.0f);
	}
	else
		output[workItemID] = (float3)(0.0f, 0.0f, 0.0f);
}