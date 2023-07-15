#pragma once
#include <CL/cl.hpp>

struct Material
{
	cl_float3 Albedo;
	cl_float3 Emission;
	cl_bool IsMetal;
	cl_bool IsTransparent;
	cl_float RefractiveIndex;
	cl_char dummy[4];
};