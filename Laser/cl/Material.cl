#ifndef MATERIAL_CL
#define MATERIAL_CL

#include "Intersection.cl"
#include "Material.cl"
#include "Random.cl"
#include "Ray.cl"
#include "Transform.cl"
#include "Triangle.cl"
#include "Vertex.cl"

typedef struct Material
{
	float3 Albedo;
	float3 Emission;
	unsigned int IsMetal;
	unsigned int IsTransparent;
	float RefractiveIndex;
	char dummy[4];
} Material;

void reflectDiffuse(Ray *ray, Intersection *isect, uint *seed)
{
	// compute two random numbers to pick a random point on the hemisphere above
	// the hitpoint
	float rand1 = 2.0f * PI * randomFloat(seed);
	float rand2 = randomFloat(seed);
	float rand2s = sqrt(rand2);

	// create a local orthogonal coordinate frame centered at the hitpoint
	float3 w = isect->N;
	float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f)
								   : (float3)(1.0f, 0.0f, 0.0f);
	float3 u = normalize(cross(axis, w));
	float3 v = cross(w, u);

	// use the coordinte frame and random numbers to compute the next ray
	// direction
	ray->dir = normalize(u * cos(rand1) * rand2s + v * sin(rand1) * rand2s +
		w * sqrt(1.0f - rand2));
	ray->orig = isect->P + isect->N * EPSILON;
}

void reflectSpecular(Ray *ray, Intersection *isect)
{
	ray->dir = normalize(ray->dir - 2.0f * dot(ray->dir, isect->N) * isect->N);
	ray->orig = isect->P + isect->N * EPSILON;
}

void refract(Ray *ray, Intersection *isect, float ratioIOR, bool frontFace)
{
	float cosTheta;
	if (frontFace)
		cosTheta = dot(-ray->dir, isect->N);
	else
		cosTheta = dot(ray->dir, isect->N);

	// Calculate perpendicular and parallel components
	float3 rOutPerp = ratioIOR * (ray->dir + cosTheta * isect->N);
	float3 rOutParallel =
		-sqrt(fabs(1.0f - pow(length(rOutPerp), 2.0f))) * isect->N;
	ray->dir = normalize(rOutPerp + rOutParallel);

	if (frontFace)
		ray->orig = isect->P - isect->N * EPSILON;
	else
		ray->orig = isect->P + isect->N * EPSILON;
}

bool useFlatShading(float3 *v0n, float3 *v1n, float3 *v2n)
{
	bool isFlat = false;
	float3 degenerateNormal = (float3)(0.0f, 0.0f, 0.0f);

	// If at least one vertex normal is degenerate (0,0,0), isFlat = true
	int3 equal = isequal(*v0n, degenerateNormal);
	isFlat |= (equal.x && equal.y && equal.z);
	equal = isequal(*v1n, degenerateNormal);
	isFlat |= (equal.x && equal.y && equal.z);
	equal = isequal(*v2n, degenerateNormal);
	isFlat |= (equal.x && equal.y && equal.z);

	return isFlat;
}

float3 computeSmoothNormal(Intersection *isect, float3 *v0n, float3 *v1n,
	float3 *v2n, mat4 *m)
{
	float3 normal;

	// Interpolate normal for smooth shading
	normal = *v0n * (1.0f - isect->u - isect->v);
	normal += *v1n * isect->u;
	normal += *v2n * isect->v;

	// Transform interpolated normal to world space
	return normalize(multMat4Normal(m, &normal));
}

float3 calcRefractionDirection(float3 *normal, float3 *incident,
	float refractiveIndexRatio)
{
	float3 n = *normal;
	float3 i = *incident;

	float cosTheta = dot(-i, n);
	float3 rOutPerp = refractiveIndexRatio * (i + cosTheta * n);
	float3 rOutParallel = -sqrt(fabs(1.0f - pow(length(rOutPerp), 2.0f))) * n;

	return normalize(rOutPerp + rOutParallel);
}

void bounceRay(Ray *ray, Intersection *isect, __global Vertex *vertices,
	__global Triangle *triangles, Material *material, __global mat4 *transforms,
	uint *seed)
{
	// Local copy of vertex normals
	float3 v0n = vertices[triangles[isect->TriangleIndex].v0].Normal;
	float3 v1n = vertices[triangles[isect->TriangleIndex].v1].Normal;
	float3 v2n = vertices[triangles[isect->TriangleIndex].v2].Normal;

	mat4 transform;

	// If using flat shading, use calculated normal from triangle intersection
	// Else compute normal for smooth shading
	if (!useFlatShading(&v0n, &v1n, &v2n))
	{
		// Local copy of transform
		transform[0] = transforms[triangles[isect->TriangleIndex].Transform][0];
		transform[1] = transforms[triangles[isect->TriangleIndex].Transform][1];
		transform[2] = transforms[triangles[isect->TriangleIndex].Transform][2];
		transform[3] = transforms[triangles[isect->TriangleIndex].Transform][3];

		isect->N = computeSmoothNormal(isect, &v0n, &v1n, &v2n, &transform);
	}

	// Process refractive materials
	if (material->IsTransparent)
	{
		bool frontFace = dot(ray->dir, isect->N) < 0.0f;
		float refractiveIndexRatio = frontFace
			? (1.0f / material->RefractiveIndex)
			: material->RefractiveIndex;

		float cosTheta;
		if (frontFace)
			cosTheta = dot(-ray->dir, isect->N);
		else
			cosTheta = dot(ray->dir, isect->N);

		float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

		// Schlick's approximation for Fresnel effect
		float r0 =
			(1.0f - refractiveIndexRatio) / (1.0f + refractiveIndexRatio);
		r0 = r0 * r0;
		float reflectance = r0 + (1.0f - r0) * pow((1.0f - cosTheta), 5.0f);

		// Total internal reflection
		if (refractiveIndexRatio * sinTheta > 1.0f ||
			reflectance > randomFloat(seed))
			reflectSpecular(ray, isect);

		// Refract
		else
			refract(ray, isect, refractiveIndexRatio, frontFace);
	}

	// Process metal
	else if (material->IsMetal)
		reflectSpecular(ray, isect);

	// Process diffuse
	else
		reflectDiffuse(ray, isect, seed);
}

#endif // MATERIAL_CL
