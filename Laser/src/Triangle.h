#pragma once
#include <CL/cl.hpp>

struct Triangle
{
	cl_uint v0;
	cl_uint v1;
	cl_uint v2;
	cl_uint Material;
	cl_uint Transform;
};