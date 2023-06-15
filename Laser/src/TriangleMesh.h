#pragma once

#include <vector>

#include <CL/cl.hpp>

#include "Triangle.h"

class TriangleMesh
{
public:
	TriangleMesh();

	const std::vector<cl_float3>* const GetVerticesPtr() const;
	const std::vector<Triangle>* const GetTrianglesPtr() const;

private:
	std::vector<cl_float3> m_Vertices;
	std::vector<Triangle> m_Triangles;
};