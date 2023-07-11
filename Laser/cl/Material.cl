#include "Random.cl"
#include "Intersection.cl"

struct Material
{
	float3 Albedo;
	float3 Emission;
	bool IsMetal;
	char dummy[12];
};

float3 calcSpecularReflectionDirection(struct Intersection* isect, float3* incident)
{
	float3 n = isect->N;
	float3 i = *incident;

	float3 dir = i - 2.0f * dot(i, n) * n;
	return dir;
}

float3 calcDiffuseReflectionDirection(struct Intersection* isect, unsigned int* seed0, unsigned int* seed1)
{
	// compute two random numbers to pick a random point on the hemisphere above the hitpoint
	float rand1 = 2.0f * PI * getRandom(seed0, seed1);
	float rand2 = getRandom(seed0, seed1);
	float rand2s = sqrt(rand2);

	// create a local orthogonal coordinate frame centered at the hitpoint
	float3 w = isect->N;
	float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
	float3 u = normalize(cross(axis, w));
	float3 v = cross(w, u);

	// use the coordinte frame and random numbers to compute the next ray direction
	float3 dir = normalize(u * cos(rand1)*rand2s + v*sin(rand1)*rand2s + w*sqrt(1.0f - rand2));
	return dir;
}
