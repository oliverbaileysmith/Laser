#pragma once
#include <CL/cl.hpp>

struct Material
{
	cl_float3 Albedo;
	cl_float3 Emission;
	cl_bool IsMetal;
	cl_char dummy[12];
};