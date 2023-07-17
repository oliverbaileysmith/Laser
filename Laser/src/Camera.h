#pragma once

#include <glm/glm.hpp>

#include "Image.h"

class Camera
{
public:
	Camera(cl_float verticalFOV, cl_float aspectRatio);

	struct Props
	{
		cl_float3 Position;
		cl_float3 Target;
		cl_float3 UpperLeftCorner;
		cl_float VerticalFOV;
		cl_float AspectRatio;
		cl_float ViewportHeight;
		cl_float ViewportWidth;
		cl_float FocalLength;
	};

	Props GetProps() const;

private:
	Camera::Props m_Props;
};