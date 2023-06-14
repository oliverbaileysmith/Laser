__constant float EPSILON = 0.00001f;
__constant float PI = 3.14159265359f;
__constant int SAMPLES = 128;

#include "Ray.cl"
#include "Triangle.cl"
#include "Material.cl"
#include "Intersection.cl"
#include "RenderStats.cl"
#include "Random.cl"

bool intersect(struct Ray* ray, __global float3* vertices,
	__global struct Triangle* triangles, unsigned int n_Triangles,
	__global struct Material* materials, float* t,	float3* n,
	struct Intersection* isect, __global struct RenderStats* renderStats)
{
	bool hit = false;
	float closestT = INFINITY;

	// test ray with each triangle
	for (int i = 0; i < n_Triangles; i++)
	{
		float3 v0 = vertices[triangles[i].v0];
		float3 v1 = vertices[triangles[i].v1];
		float3 v2 = vertices[triangles[i].v2];

		if (intersectTriangle(ray, v0, v1, v2, &closestT, n, renderStats))
		{
			hit = true;
			// update t if closer hit
			if (closestT != 0.0f && closestT < *t)
			{
				*t = closestT;
				isect->P = ray->orig + *t * ray->dir;
				isect->N = *n;
				isect->Albedo = materials[triangles[i].MaterialIndex].Albedo;
				isect->Emission = materials[triangles[i].MaterialIndex].Emission;
			}
		}
	}
	return hit;
}

float3 trace(struct Ray* primaryRay, __global float3* vertices, __global struct Triangle* triangles,
	unsigned int n_Triangles, __global struct Material* materials, __global struct RenderStats* renderStats,
	unsigned int* seed0, unsigned int* seed1)
{
	float3 color = (float3)(0.0f, 0.0f, 0.0f);
	float3 mask = (float3)(1.0f, 1.0f, 1.0f);

	// Create local copy of primary ray for this sample as it will be modified at each depth level
	struct Ray ray = *primaryRay;

	for (int depth = 0; depth < 8; depth++)
	{
		float t = INFINITY;
		float3 n;
		struct Intersection isect;

		// test ray with scene
		if (!intersect(&ray, vertices, triangles, n_Triangles, materials, &t, &n, &isect, renderStats))
		{
			// return background color if miss
			return color += mask * (float3)(0.2f, 0.2f, 0.2f);
		}

		// modify ray for diffuse reflection
		/* compute two random numbers to pick a random point on the hemisphere above the hitpoint*/
		float rand1 = 2.0f * PI * getRandom(seed0, seed1);
		float rand2 = getRandom(seed0, seed1);
		float rand2s = sqrt(rand2);

		/* create a local orthogonal coordinate frame centered at the hitpoint */
		float3 w = isect.N;
		float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
		float3 u = normalize(cross(axis, w));
		float3 v = cross(w, u);

		/* use the coordinte frame and random numbers to compute the next ray direction */
		float3 newDir = normalize(u * cos(rand1)*rand2s + v*sin(rand1)*rand2s + w*sqrt(1.0f - rand2));

		ray.orig = isect.P + isect.N * EPSILON;
		ray.dir = newDir;

		// accumulate color
		color += mask * isect.Emission;
		mask *= isect.Albedo;
		mask *= dot(ray.dir, isect.N);
	}
	return color;
}

__kernel void Laser(__global float3* output, int imageWidth, int imageHeight,
	float aspectRatio, float viewportWidth, float viewportHeight,
	float focalLength, float3 cameraOrigin, float3 upperLeftCorner,
	__global float3* vertices, __global struct Triangle* triangles,
	unsigned int n_Triangles, __global struct Material* materials,
	__global struct RenderStats* renderStats)
{
	// calculate pixel coordinates
	const unsigned int workItemID = get_global_id(0);
	unsigned int x = workItemID % imageWidth;
	unsigned int y = workItemID / imageWidth;
	float fx = (float)x / (float)(imageWidth - 1);
	float fy = (float)y / (float)(imageHeight - 1);

	// seeds for random
	unsigned int seed0 = x;
	unsigned int seed1 = y;

	// generate primary ray
	//atomic_inc(&(renderStats->n_PrimaryRays));
	struct Ray primaryRay;
	primaryRay.orig = cameraOrigin;
	primaryRay.dir = upperLeftCorner;
	primaryRay.dir.x += fx * viewportWidth;
	primaryRay.dir.y -= fy * viewportHeight;
	primaryRay.dir -= cameraOrigin;
	primaryRay.dir = normalize(primaryRay.dir);

	float3 color = (float3)(0.0f, 0.0f, 0.0f);
	float invSamples = 1.0f / SAMPLES;

	for (int i = 0; i < SAMPLES; i++)
		color += trace(&primaryRay, vertices, triangles, n_Triangles, materials, renderStats, &seed0, &seed1) * invSamples;
	output[workItemID] = color;
}