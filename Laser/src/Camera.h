#pragma once

#include <glm/glm.hpp>

#include "Image.h"

class Camera
{
public:
	Camera(cl_float3 position, cl_float3 target, cl_float verticalFOV,
		cl_float aspectRatio, cl_float aperture, cl_float focusDistance);

	struct Props
	{
		cl_float3 Position;
		cl_float3 Target;
		cl_float3 UpperLeftCorner;
		cl_float3 ViewportHorizontal;
		cl_float3 ViewportVertical;
		cl_float3 u; // horizontal axis for camera basis
		cl_float3 v; // vertical axis for camera basis
		cl_float3 w; // forward axis for camera basis
		cl_float VerticalFOV;
		cl_float AspectRatio;
		cl_float LensRadius;
	};

	Props GetProps() const;

private:
	Camera::Props m_Props;
};
