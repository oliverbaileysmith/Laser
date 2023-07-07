#include "BVH.h"

BVH::BVH(const std::vector<cl_float3>& vertices, const std::vector<Triangle>& triangles, const std::vector<glm::mat4>& transforms)
	: m_Vertices(vertices), m_Triangles(triangles), m_Transforms(transforms)
{
}

BVH::~BVH()
{
}