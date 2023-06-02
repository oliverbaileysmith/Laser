struct Ray
{
	float3 orig;
	float3 dir;
};

struct Triangle
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

bool intersectTriangle(struct Ray* ray, float3 v0, float3 v1, float3 v2,
	float* t, __global struct RenderStats* renderStats)
{
	atomic_inc(&(renderStats->n_RayTriangleTests));

	// calculate triangle/plane normal
	float3 n;
	float3 v0v1 = v1 - v0;
	float3 v0v2 = v2 - v0;
	n = cross(v0v1, v0v2);

	// calculate intersection point of ray and the plane in which the triangle lies
	float D = - dot(n, v0); // distance from origin to plane parallel normal

	float denom = dot(n, ray->dir);
    if (fabs(denom) < 0.00001f) // test for ray parallel to triangle
        return false;

	*t = -(dot(n, ray->orig) + D) / denom;
	if (*t < 0.0f) // test if hit point is behind camera
		return false;

	float3 p = ray->orig + *t * ray->dir; // intersection point

	// check if intersection point is inside triangle
	float3 C;
	
	float3 edge0 = v1 - v0; // first edge
	float3 v0p = p - v0;
	C = cross(edge0, v0p);
	if (dot(n, C) < 0.0f)
		return false;

	float3 edge1 = v2 - v1; // second edge
	float3 v1p = p - v1;
	C = cross(edge1, v1p);
	if (dot(n, C) < 0.0f)
		return false;

	float3 edge2 = v0 - v2; // third edge
	float3 v2p = p - v2;
	C = cross(edge2, v2p);
	if (dot(n, C) < 0.0f)
		return false;

	atomic_inc(&(renderStats->n_RayTriangleIsects));
	return true;
}

bool intersect(struct Ray* ray, __global float3* vertices,
	__global struct Triangle* triangles, unsigned int n_Triangles, float* t,
	__global struct RenderStats* renderStats)
{
	bool hit = false;
	float closestT = INFINITY;

	// test ray with each triangle
	for (int i = 0; i < n_Triangles; i++)
	{
		float3 v0 = vertices[triangles[i].v0];
		float3 v1 = vertices[triangles[i].v1];
		float3 v2 = vertices[triangles[i].v2];
		if (intersectTriangle(ray, v0, v1, v2, &closestT, renderStats))
			hit = true;
	}

	*t = closestT;
	return hit;
}

__kernel void Laser(__global float3* output, int imageWidth, int imageHeight,
	float aspectRatio, float viewportWidth, float viewportHeight,
	float focalLength, float3 cameraOrigin, float3 upperLeftCorner,
	__global float3* vertices, __global struct Triangle* triangles,
	unsigned int n_Triangles, __global struct RenderStats* renderStats )
{
	// calculate pixel coordinates
	const int workItemID = get_global_id(0);
	int x = workItemID % imageWidth;
	int y = workItemID / imageWidth;
	float fx = (float)x / (float)(imageWidth - 1);
	float fy = (float)y / (float)(imageHeight - 1);

	// generate ray
	atomic_inc(&(renderStats->n_PrimaryRays));
	struct Ray ray;
	ray.orig = cameraOrigin;
	ray.dir = upperLeftCorner;
	ray.dir.x += fx * viewportWidth;
	ray.dir.y -= fy * viewportHeight;
	ray.dir -= cameraOrigin;
	ray.dir = normalize(ray.dir);

	float t = INFINITY;

	// test ray with scene
	if (intersect(&ray, vertices, triangles, n_Triangles, &t, renderStats))
	{
		output[workItemID] = (float3)(1.0f, 0.0f, 0.0f);
	}
	else
		output[workItemID] = (float3)(0.0f, 0.0f, 0.0f);
}