#pragma once

#include <vector>

#include <CL/cl.hpp>

#include "Vertex.h"
#include "Triangle.h"

class TriangleMesh
{
public:
	TriangleMesh(std::vector<Vertex> vertices, std::vector<Triangle> triangles);

	std::vector<Vertex> m_Vertices;
	std::vector<Triangle> m_Triangles;
};