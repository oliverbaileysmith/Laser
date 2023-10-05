#ifndef CAMERA_CL
#define CAMERA_CL

#include "Random.cl"
#include "Ray.cl"

typedef struct CameraProps
{
	float3 Position;
	float3 Target;
	float3 UpperLeftCorner;
	float3 ViewportHorizontal;
	float3 ViewportVertical;
	float3 u;
	float3 v;
	float3 w;
	float VerticalFOV;
	float AspectRatio;
	float LensRadius;
} CameraProps;

Ray generateRay(__global CameraProps *camera, float fx, float fy, uint *seed)
{
	float3 pointInLens = camera->LensRadius * randomFloat3InUnitDisk(seed);
	float3 offset = camera->u * pointInLens.x + camera->v * pointInLens.y;

	Ray ray;
	ray.orig = camera->Position + offset;

	ray.dir = camera->UpperLeftCorner;
	ray.dir += fx * camera->ViewportHorizontal;
	ray.dir -= fy * camera->ViewportVertical;
	ray.dir -= camera->Position;
	ray.dir -= offset;
	ray.dir = normalize(ray.dir);

	return ray;
}

#endif // CAMERA_CL
