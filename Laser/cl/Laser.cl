__constant float EPSILON = 0.00001f;
__constant float PI = 3.14159265359f;
__constant unsigned int SAMPLES = 32;
__constant unsigned int MAX_DEPTH = 4;

#include "Ray.cl"
#include "Triangle.cl"
#include "Material.cl"
#include "Transform.cl"
#include "BVH.cl"

float3 trace(struct Ray* primaryRay, __global float3* vertices,
	__global struct Triangle* triangles, unsigned int n_Triangles,
	__global struct Material* materials, __global mat4* transforms,
	__global struct BVHLinearNode* bvh,	__global struct RenderStats* renderStats,
	unsigned int* seed0, unsigned int* seed1)
{
	float3 color = (float3)(0.0f, 0.0f, 0.0f);
	float3 mask = (float3)(1.0f, 1.0f, 1.0f);

	// Create local copy of primary ray for this sample as it will be modified at each depth level
	struct Ray ray = *primaryRay;

	for (int depth = 0; depth < MAX_DEPTH; depth++)
	{
		float t = INFINITY;
		float3 n;
		struct Intersection isect;

		if (!intersectBVH(&ray, vertices, triangles, materials, transforms, bvh, &t, &n, &isect, renderStats))
			// Return background color
			return (float3)(0.2f, 0.2f, 0.2f);

		// Local copy of material
		struct Material material = materials[isect.MaterialIndex];
		
		// Update ray direction for next bounce using specular reflection
		if (material.IsMetal)
			ray.dir = calcSpecularReflectionDirection(&isect, &ray.dir);

		// Update ray direction for next bounce using diffuse reflection
		else
			ray.dir = calcDiffuseReflectionDirection(&isect, seed0, seed1);\

		// Update ray origin for next bounce
		ray.orig = isect.P + isect.N * EPSILON;

		// accumulate color
		color += mask * material.Emission;
		mask *= material.Albedo;
		mask *= dot(ray.dir, isect.N);
	}
	return color;
}

__kernel void Laser(__global float3* output, unsigned int imageWidth,
	unsigned int imageHeight, float aspectRatio, float viewportWidth,
	float viewportHeight, float focalLength, float3 cameraOrigin,
	float3 upperLeftCorner,	__global float3* vertices,
	__global struct Triangle* triangles, unsigned int n_Triangles,
	__global struct Material* materials, __global mat4* transforms,
	__global struct BVHLinearNode* bvh, __global struct RenderStats* renderStats,
	unsigned int xOffset, unsigned int yOffset,
	unsigned int tileWidth, unsigned int tileHeight)
{
	// Calculate pixel coordinates
	const unsigned int workItemID = get_global_id(0);
	unsigned int x = xOffset + (workItemID % tileWidth);
	unsigned int y = yOffset + (workItemID / tileWidth);

	// Don't trace ray if pixel is not in image bounds
	// This happens in right column and bottom row of tiles
	if (x >= imageWidth || y >= imageHeight) return;

	float fx = (float)x / (float)(imageWidth - 1);
	float fy = (float)y / (float)(imageHeight - 1);

	// Seeds for random
	unsigned int seed0 = x;
	unsigned int seed1 = y;

	// Generate primary ray
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
		color += trace(&primaryRay, vertices, triangles, n_Triangles, materials, transforms, bvh, renderStats, &seed0, &seed1) * invSamples;
	output[workItemID] = color;
}