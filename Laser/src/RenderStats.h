#pragma once

#include <CL/cl.hpp>

struct RenderStats
{
	cl_uint n_PrimaryRays = 0;
	cl_uint n_RayTriangleTests = 0;
	cl_uint n_RayTriangleIsects = 0;
	cl_float RenderTime = 0.0f; // Time in seconds
};
