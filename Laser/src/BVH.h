#pragma once

#include <CL/cl.hpp>
#include <glm/glm.hpp>

#include "Triangle.h"
#include "Bounds.h"

class BVH
{
public:
	BVH(const std::vector<cl_float3>& vertices, const std::vector<Triangle>& triangles, const std::vector<glm::mat4>& transforms);
	~BVH();

private:
	// BVH node for building
	struct BVHBuildNode
	{
		Bounds Bounds;
		BVHBuildNode* Children[2];
		cl_uint FirstTriangle;
		cl_uint nTriangles;
	};

	// Scene data
	std::vector <cl_float3> m_Vertices;
	std::vector<Triangle> m_Triangles;
	std::vector<glm::mat4> m_Transforms;
};