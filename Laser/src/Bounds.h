#pragma once

#include <CL/cl.hpp>

#include "Triangle.h"

struct Bounds
{
public:
	cl_float3 pMin;
	cl_float3 pMax;
	
	Bounds();
	// Compute triangle bounds
	Bounds(const cl_float3& v0, const cl_float3& v1, const cl_float3& v2);

	// Compute bounds from 2 points
	Bounds(const cl_float3& p0, const cl_float3& p1);
	// Extend bounds by a point
	void Extend(const cl_float3& p);
	// Join two bounds
	void Join(const Bounds& b);
};