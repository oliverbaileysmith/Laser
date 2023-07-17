__constant float EPSILON = 0.00001f;
__constant float PI = 3.14159265359f;
__constant unsigned int SAMPLES = 64;
__constant unsigned int MAX_DEPTH = 16;

#include "Image.cl"
#include "Camera.cl"
#include "Ray.cl"
#include "Triangle.cl"
#include "Material.cl"
#include "BVH.cl"

float3 traceDebug(Ray* primaryRay, __global Vertex* vertices, __global Triangle* triangles,
	__global Material* materials, __global mat4* transforms, __global BVHLinearNode* bvh,
	__global RenderStats* renderStats, unsigned int* seed0,	unsigned int* seed1)
{
	Ray ray = *primaryRay;
	float t = INFINITY;
	float3 n;
	Intersection isect;

	if (!intersectBVH(&ray, vertices, triangles, materials, transforms, bvh, &t, &n, &isect, renderStats))
		// Return background color
		return (float3)(0.2f, 0.2f, 0.2f);

	//return (float3) (isect.u, isect.v, 1.0f - isect.u - isect.v); // visualize barycentric coords

	mat4 transform;
	transform[0] = transforms[triangles[isect.TriangleIndex].Transform][0];
	transform[1] = transforms[triangles[isect.TriangleIndex].Transform][1];
	transform[2] = transforms[triangles[isect.TriangleIndex].Transform][2];
	transform[3] = transforms[triangles[isect.TriangleIndex].Transform][3];

	float3 v0n = vertices[triangles[isect.TriangleIndex].v0].Normal;
	float3 v1n = vertices[triangles[isect.TriangleIndex].v1].Normal;
	float3 v2n = vertices[triangles[isect.TriangleIndex].v2].Normal;

	float3 shadingNormal = v0n * (1.0f - isect.u - isect.v);
	shadingNormal += v1n * isect.u;
	shadingNormal += v2n * isect.v;

	shadingNormal = normalize(multMat4Normal(&transform, &shadingNormal));

	return shadingNormal * 0.5f + 0.5f; // visualize normals
}

float3 trace(Ray* primaryRay, __global Vertex* vertices, __global Triangle* triangles,
	__global Material* materials, __global mat4* transforms, __global BVHLinearNode* bvh,
	__global RenderStats* renderStats, unsigned int* seed0, unsigned int* seed1)
{
	float3 color = (float3)(0.0f, 0.0f, 0.0f);
	float3 mask = (float3)(1.0f, 1.0f, 1.0f);

	// Create local copy of primary ray for this sample as it will be modified at each depth level
	Ray ray = *primaryRay;

	for (int depth = 0; depth < MAX_DEPTH; depth++)
	{
		float t = INFINITY;
		float3 n;
		Intersection isect;

		if (!intersectBVH(&ray, vertices, triangles, materials, transforms, bvh, &t, &n, &isect, renderStats))
			// Return background color
			return (float3)(0.2f, 0.2f, 0.2f);
		
		// Local copy of material
		Material material = materials[triangles[isect.TriangleIndex].Material];

		bounceRay(&ray, &isect, vertices, triangles, &material, transforms, seed0, seed1);

		// Accumulate color		
		color += mask * material.Emission;
		mask *= material.Albedo;
		
		// Cosine-weighted importance sampling for diffuse
		if (!material.IsTransparent && !material.IsMetal)
			mask *= dot(ray.dir, isect.N);
	}
	return color;
}

__kernel void Laser(__global float3* output, __global ImageProps* image,
	__global CameraProps* camera, __global Vertex* vertices, __global Triangle* triangles,
	__global Material* materials, __global mat4* transforms, __global BVHLinearNode* bvh,
	__global RenderStats* renderStats, unsigned int xOffset, unsigned int yOffset)
{	
	// Calculate pixel coordinates
	const unsigned int workItemID = get_global_id(0);
	unsigned int x = xOffset + (workItemID % image->TileWidth);
	unsigned int y = yOffset + (workItemID / image->TileWidth);

	// Don't trace ray if pixel is not in image bounds
	// This happens in right column and bottom row of tiles
	if (x >= image->Width || y >= image->Height) return;

	float fx = (float)x / (float)(image->Width - 1);
	float fy = (float)y / (float)(image->Height - 1);

	// Seeds for random
	unsigned int seed0 = x;
	unsigned int seed1 = y;

	// Generate primary ray
	//atomic_inc(&(renderStats->n_PrimaryRays));
	Ray primaryRay;
	primaryRay.orig = camera->Position;
	primaryRay.dir = camera->UpperLeftCorner;
	primaryRay.dir += fx * camera->ViewportHorizontal;
	primaryRay.dir -= fy * camera->ViewportVertical;
	primaryRay.dir -= camera->Position;
	primaryRay.dir = normalize(primaryRay.dir);

	float3 color = (float3)(0.0f, 0.0f, 0.0f);
	float invSamples = 1.0f / SAMPLES;

	for (int i = 0; i < SAMPLES; i++)
		color += trace(&primaryRay, vertices, triangles, materials, transforms, bvh, renderStats, &seed0, &seed1) * invSamples;
	output[workItemID] = color;

	//output[workItemID] = traceDebug(&primaryRay, vertices, triangles, materials, transforms, bvh, renderStats, &seed0, &seed1);
}