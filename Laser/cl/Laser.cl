__constant float EPSILON = 0.00001f;
__constant float PI = 3.14159265359f;
__constant unsigned int SAMPLES = 64;
__constant unsigned int MAX_DEPTH = 8;

#include "Ray.cl"
#include "Triangle.cl"
#include "Material.cl"
#include "Transform.cl"
#include "BVH.cl"

float3 tracedebug(struct Ray* primaryRay, __global struct Vertex* vertices,
	__global struct Triangle* triangles, unsigned int n_Triangles,
	__global struct Material* materials, __global mat4* transforms,
	__global struct BVHLinearNode* bvh,	__global struct RenderStats* renderStats,
	unsigned int* seed0, unsigned int* seed1)
{
	struct Ray ray = *primaryRay;
	float t = INFINITY;
	float3 n;
	struct Intersection isect;

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

float3 trace(struct Ray* primaryRay, __global struct Vertex* vertices,
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
		struct Material material = materials[triangles[isect.TriangleIndex].Material];

		// Update ray direction for refraction
		if (material.IsGlass)
		{
			bool frontFace = dot(ray.dir, isect.N) < 0.0f;
			float refractiveIndexRatio = frontFace ? (1.0f / material.RefractiveIndex) : material.RefractiveIndex;
				
			float cosTheta = dot(-ray.dir, isect.N);
			float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

            // Schlick's approximation for Fresnel effect
            //float r0 = (1.0f - refractiveIndexRatio) / (1.0f + refractiveIndexRatio);
            //r0 = r0*r0;
            //float reflectance = r0 + (1.0f - r0) * pow((1.0f - cosTheta), 5.0f);

			// Total internal reflection
			if (refractiveIndexRatio * sinTheta > 1.0f /*|| reflectance > getRandom(seed0, seed1)*/)
			{
				ray.dir = normalize(ray.dir - 2.0f * dot(ray.dir, isect.N) * isect.N);
				ray.orig = isect.P + isect.N * EPSILON;
			}

			// Refract
			else
			{
				if (frontFace)
				{
					ray.dir = calcRefractionDirection(&isect, &ray.dir, refractiveIndexRatio);
					ray.orig = isect.P - isect.N * EPSILON;
				}
				else
				{
					ray.dir = -calcRefractionDirection(&isect, &ray.dir, refractiveIndexRatio);
					ray.orig = isect.P + isect.N * EPSILON;
				}
			}

			// accumulate color
			color += mask * material.Emission;
			mask *= material.Albedo;
		}

		// Update ray direction for next bounce using specular reflection
		else if (material.IsMetal)
		{
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

			ray.dir = calcSpecularReflectionDirection(&shadingNormal, &ray.dir);
			ray.orig = isect.P + shadingNormal * EPSILON;

			// accumulate color
			color += mask * material.Emission;
			mask *= material.Albedo;
			mask *= dot(ray.dir, shadingNormal);
		}

		// Update ray for next bounce using diffuse reflection
		else
		{
			ray.dir = calcDiffuseReflectionDirection(&isect, seed0, seed1);
			ray.orig = isect.P + isect.N * EPSILON;

			// accumulate color
			color += mask * material.Emission;
			mask *= material.Albedo;
			mask *= dot(ray.dir, isect.N);
		}
	}
	return color;
}

__kernel void Laser(__global float3* output, unsigned int imageWidth,
	unsigned int imageHeight, float aspectRatio, float viewportWidth,
	float viewportHeight, float focalLength, float3 cameraOrigin,
	float3 upperLeftCorner,	__global struct Vertex* vertices,
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

	//output[workItemID] = tracedebug(&primaryRay, vertices, triangles, n_Triangles, materials, transforms, bvh, renderStats, &seed0, &seed1);
}