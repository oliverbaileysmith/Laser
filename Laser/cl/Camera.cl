#include "Ray.cl"

typedef struct CameraProps
{
	float3 Position;
	float3 Target;
	float3 UpperLeftCorner;
	float3 ViewportHorizontal;
	float3 ViewportVertical;
	float VerticalFOV;
	float AspectRatio;
	float FocalLength;
} CameraProps;

Ray generateRay(__global CameraProps* camera, float fx, float fy)
{
	Ray ray;
	ray.orig = camera->Position;
	
	ray.dir = camera->UpperLeftCorner;
	ray.dir += fx * camera->ViewportHorizontal;
	ray.dir -= fy * camera->ViewportVertical;
	ray.dir -= camera->Position;
	ray.dir = normalize(ray.dir);
	
	return ray;
}