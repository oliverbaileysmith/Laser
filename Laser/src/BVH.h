#pragma once

#include <CL/cl.hpp>
#include <glm/glm.hpp>

#include "Triangle.h"
#include "Bounds.h"

class BVH
{
private:
	/********** BVH TRIANGLE INFO **********/
	struct BVHTriangleInfo
	{
	public:
		BVHTriangleInfo(cl_uint triangleNumber, const ::Bounds& bounds);

	public:
		cl_uint TriangleNumber;
		Bounds Bounds;
		cl_float3 Centroid;

	private:
		cl_float3 CalcCentroid(const ::Bounds& bounds);
	};

	/********** BVH BUILD NODE **********/
	struct BVHBuildNode
	{
	public:
		Bounds Bounds;
		BVHBuildNode* Children[2];
		cl_uint SplitAxis;
		cl_uint FirstTriangle;
		cl_uint nTriangles;

	public:
		BVHBuildNode();
		~BVHBuildNode();

		void InitLeaf(cl_uint firstTriangle, cl_uint nTriangles, const ::Bounds& bounds);
		void InitInterior(cl_uint axis, BVHBuildNode* child0, BVHBuildNode* child1);
	};

	/********** BVH LINEAR NODE **********/
	struct BVHLinearNode
	{
		Bounds Bounds;
		cl_uint SecondChildOffset; // First child is next node in array
		cl_uint FirstTriangle;
		cl_uint nTriangles;
		cl_uint SplitAxis;
	};

	/********** BVH **********/
public:
	BVH(const std::vector<cl_float3>& vertices, const std::vector<Triangle>& triangles, const std::vector<glm::mat4>& transforms);
	~BVH();

private:
	BVHBuildNode* Build(std::vector<BVHTriangleInfo>& trianglesInfo, cl_uint start,
		cl_uint end, cl_uint* totalNodes, std::vector<Triangle>& orderedTriangles);

	cl_uint Flatten(BVHBuildNode* node, cl_uint* offset);

	Bounds CalcTriangleBounds(cl_uint triangle) const;

private:
	// Scene data
	std::vector <cl_float3> m_Vertices;
	std::vector<Triangle> m_Triangles;
	std::vector<glm::mat4> m_Transforms;
	std::vector<BVHLinearNode> m_BVHLinearNodes;
};