#include "Random.cl"
#include "Intersection.cl"

struct Material
{
	float3 Albedo;
	float3 Emission;
	unsigned int IsMetal;
	unsigned int IsGlass;
	float RefractiveIndex; 
	char dummy[4];
};

float3 calcSpecularReflectionDirection(float3* normal, float3* incident)
{
	float3 n = *normal;
	float3 i = *incident;

	return normalize(i - 2.0f * dot(i, n) * n);
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
	return normalize(u * cos(rand1)*rand2s + v*sin(rand1)*rand2s + w*sqrt(1.0f - rand2));
}

float3 calcRefractionDirection(struct Intersection* isect, float3* incident, float refractiveIndexRatio)
{
	float3 n = isect->N;
	float3 i = *incident;

	float cosTheta = dot(-i, n);
	float3 rOutPerp = refractiveIndexRatio * (i + cosTheta * n);
	float3 rOutParallel = -sqrt(fabs(1.0f - pow(length(rOutPerp), 2.0f))) * n;

	return normalize(rOutPerp + rOutParallel);
}