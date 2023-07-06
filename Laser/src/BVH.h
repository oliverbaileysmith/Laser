#pragma once

#include <CL/cl.hpp>
#include <glm/glm.hpp>

#include "Triangle.h"
#include "Bounds.h"

class BVH
{
public:
	BVH(const std::vector<cl_float3>& vertices, const std::vector<Triangle>& triangles, const std::vector<glm::mat4>& transforms);

	Bounds GetSceneBounds() const;

private:
	Bounds GetTriangleBounds(cl_uint index) const;
	cl_float3 GetCentroid(const Bounds& triangleBounds) const;

private:
	// Octree node
	struct OctreeNode
	{
	public:
		OctreeNode* Children[8];
		Bounds NodeBounds;
		cl_bool IsLeaf;
		std::vector<cl_uint> Triangles; // indices into scene's triangles array
		uint32_t Depth = 0;

		OctreeNode(const Bounds& bounds);
		~OctreeNode();
	};

	// Octree used to build BVH by spatially grouping triangles
	class Octree
	{
	public:
		Octree(BVH* bvh, cl_uint maxDepth = 32);
		~Octree();

		void Insert(cl_uint triangle);
		void Build();

	private:
		Bounds GetChildBounds(cl_char index, const cl_float3& nodeCentroid, const Bounds& bounds) const;
		Bounds GetTriangleBounds(cl_uint index) const;
		cl_float3 GetCentroid(const Bounds& triangleBounds) const;

		void Insert(OctreeNode* node, cl_uint triangle, const Bounds& bounds, uint32_t depth);

	private:
		OctreeNode* m_Root;
		BVH* m_BVH;
		Bounds m_Bounds;
		cl_uint m_MaxDepth;
	};

	// Scene data
	Bounds m_Bounds;
	std::vector <cl_float3> m_Vertices;
	std::vector<Triangle> m_Triangles;
	std::vector<glm::mat4> m_Transforms;
};