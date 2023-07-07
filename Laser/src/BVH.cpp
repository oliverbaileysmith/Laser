#include "BVH.h"
#include <algorithm>

/********** BVH TRIANGLE INFO **********/

BVH::BVHTriangleInfo::BVHTriangleInfo(cl_uint triangleNumber, const ::Bounds& bounds)
	: TriangleNumber(triangleNumber), Bounds(bounds), Centroid(CalcCentroid(bounds))
{
}

cl_float3 BVH::BVHTriangleInfo::CalcCentroid(const ::Bounds& bounds)
{
	cl_float x = (bounds.pMin.x + bounds.pMax.x) * 0.5f;
	cl_float y = (bounds.pMin.y + bounds.pMax.y) * 0.5f;
	cl_float z = (bounds.pMin.z + bounds.pMax.z) * 0.5f;

	cl_float3 result = { x, y, z };
	return result;
}

/********** BVH BUILD NODE **********/

BVH::BVHBuildNode::BVHBuildNode()
{
	Children[0] = nullptr;
	Children[1] = nullptr;
}

BVH::BVHBuildNode::~BVHBuildNode()
{
	if (Children[0]) delete Children[0];
	if (Children[1]) delete Children[1];
}

void BVH::BVHBuildNode::InitLeaf(cl_uint first, cl_uint n, const ::Bounds& bounds)
{
	FirstTriangle = first;
	this->nTriangles = n;
	Bounds = bounds;
}

void BVH::BVHBuildNode::InitInterior(cl_uint axis, BVHBuildNode* child0, BVHBuildNode* child1)
{
	Children[0] = child0;
	Children[1] = child1;
	::Bounds nodeBounds = child0->Bounds;
	nodeBounds.Join(child1->Bounds);
	Bounds = nodeBounds;
	SplitAxis = axis;
	nTriangles = 0;
}

/********** BVH **********/

BVH::BVH(const std::vector<cl_float3>& vertices, const std::vector<Triangle>& triangles, const std::vector<glm::mat4>& transforms)
	: m_Vertices(vertices), m_Triangles(triangles), m_Transforms(transforms)
{
	// Ensure at least one triangle in the scene
	if (m_Triangles.size() == 0) return;

	// Calculate scene triangle info (bounds, centroids)
	std::vector<BVHTriangleInfo> trianglesInfo;
	trianglesInfo.reserve(m_Triangles.size());
	for (uint32_t i = 0; i < m_Triangles.size(); i++)
		trianglesInfo.emplace_back(BVHTriangleInfo(i, CalcTriangleBounds(i)));

	// Build BVH tree
	uint32_t totalNodes = 0;
	std::vector<Triangle> orderedTriangles;
	orderedTriangles.reserve(m_Triangles.size());
	BVHBuildNode* root = Build(trianglesInfo, 0, m_Triangles.size(), &totalNodes, orderedTriangles);

	// Store ordered triangles
	m_Triangles.swap(orderedTriangles);
}

BVH::~BVH()
{
}

BVH::BVHBuildNode* BVH::Build(std::vector<BVHTriangleInfo>& trianglesInfo, cl_uint start,
	cl_uint end, cl_uint* totalNodes, std::vector<Triangle>& orderedTriangles)
{
	BVHBuildNode* node = new BVHBuildNode;
	(*totalNodes)++;

	// Compute bounds of triangles in node
	Bounds nodeBounds;
	for (cl_uint i = start; i < end; i++)
		nodeBounds.Join(trianglesInfo[i].Bounds);

	// If 1 triangle in node
	cl_uint nTriangles = end - start;
	if (nTriangles == 1)
	{
		// Create leaf node
		cl_uint firstTriangleOffset = orderedTriangles.size();
		for (cl_uint i = start; i < end; i++)
		{
			cl_uint triangleNumber = trianglesInfo[i].TriangleNumber;
			orderedTriangles.emplace_back(m_Triangles[triangleNumber]);
		}
		node->InitLeaf(firstTriangleOffset, nTriangles, nodeBounds);
		return node;
	}

	// If more than 1 triangle in node
	else
	{
		// Choose split dimension
		Bounds centroidBounds;
		for (cl_uint i = start; i < end; ++i)
			centroidBounds.Extend(trianglesInfo[i].Centroid);
		cl_uint dimension = centroidBounds.GetLargestDimension();
		
		// Partition triangles
		cl_uint mid = (start + end) / 2;

		cl_float pMinInDim = dimension == 0 ? centroidBounds.pMin.x : dimension == 1 ? centroidBounds.pMin.y : centroidBounds.pMin.z;
		cl_float pMaxInDim = dimension == 0 ? centroidBounds.pMax.x : dimension == 1 ? centroidBounds.pMax.y : centroidBounds.pMax.z;
		
		// If bounds of centroids of triangles is degenerate
		if (pMinInDim == pMaxInDim)
		{
			// Create leaf node
			cl_uint firstTriangleOffset = orderedTriangles.size();
			for (cl_uint i = start; i < end; i++)
			{
				cl_uint triangleNumber = trianglesInfo[i].TriangleNumber;
				orderedTriangles.emplace_back(m_Triangles[triangleNumber]);
			}
			node->InitLeaf(firstTriangleOffset, nTriangles, nodeBounds);
			return node;
		}
		else
		{
			// Partition primitives into equal subsets
			std::nth_element(&trianglesInfo[start], &trianglesInfo[mid], &trianglesInfo[end - 1] + 1,
				[dimension](const BVHTriangleInfo& a, const BVHTriangleInfo& b)
			{
				cl_float aCentroidDim = dimension == 0 ? a.Centroid.x : dimension == 1 ? a.Centroid.y : a.Centroid.z;
				cl_float bCentroidDim = dimension == 0 ? b.Centroid.x : dimension == 1 ? b.Centroid.y : b.Centroid.z;
				return aCentroidDim < bCentroidDim;
			});

			// Create interior node and recurse
			node->InitInterior(dimension,
				Build(trianglesInfo, start, mid, totalNodes, orderedTriangles),
				Build(trianglesInfo, mid, end, totalNodes, orderedTriangles));
		}
	}
	return node;
}

Bounds BVH::CalcTriangleBounds(cl_uint tri) const
{
	cl_float3 v0 = m_Vertices[m_Triangles[tri].v0];
	cl_float3 v1 = m_Vertices[m_Triangles[tri].v1];
	cl_float3 v2 = m_Vertices[m_Triangles[tri].v2];

	glm::vec3 glmv0 = { v0.x, v0.y, v0.z };
	glm::vec3 glmv1 = { v1.x, v1.y, v1.z };
	glm::vec3 glmv2 = { v2.x, v2.y, v2.z };

	glm::mat4 transform = m_Transforms[m_Triangles[tri].Transform];

	glmv0 = transform * glm::vec4(glmv0, 1.0f);
	glmv1 = transform * glm::vec4(glmv1, 1.0f);
	glmv2 = transform * glm::vec4(glmv2, 1.0f);

	v0 = { glmv0.x, glmv0.y, glmv0.z };
	v1 = { glmv1.x, glmv1.y, glmv1.z };
	v2 = { glmv2.x, glmv2.y, glmv2.z };

	return Bounds(v0, v1, v2);
}