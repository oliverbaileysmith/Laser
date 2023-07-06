#include "BVH.h"

/********** BVH **********/

BVH::BVH(const std::vector<cl_float3>& vertices, const std::vector<Triangle>& triangles, const std::vector<glm::mat4>& transforms)
	: m_Vertices(vertices), m_Triangles(triangles), m_Transforms(transforms)
{
	// Compute world bounds for entire scene
	Bounds sceneBounds;

	for (int i = 0; i < m_Triangles.size(); i++)
		sceneBounds.Join(GetTriangleBounds(i));

	m_Bounds = sceneBounds;

	// Create octree to spatially partition triangles
	Octree octree(this);

	for (int i = 0; i < m_Triangles.size(); i++)
	{
		octree.Insert(i);
	}
}

Bounds BVH::GetSceneBounds() const
{
	return m_Bounds;
}

Bounds BVH::GetTriangleBounds(cl_uint index) const
{
	Triangle triangle = m_Triangles[index];
	glm::mat4 transform = m_Transforms[triangle.Transform];
	glm::vec3 glmv0 = transform * glm::vec4(m_Vertices[triangle.v0].x, m_Vertices[triangle.v0].y, m_Vertices[triangle.v0].z, 1.0f);
	glm::vec3 glmv1 = transform * glm::vec4(m_Vertices[triangle.v1].x, m_Vertices[triangle.v1].y, m_Vertices[triangle.v1].z, 1.0f);
	glm::vec3 glmv2 = transform * glm::vec4(m_Vertices[triangle.v2].x, m_Vertices[triangle.v2].y, m_Vertices[triangle.v2].z, 1.0f);

	cl_float3 v0 = { glmv0.x, glmv0.y, glmv0.z };
	cl_float3 v1 = { glmv1.x, glmv1.y, glmv1.z };
	cl_float3 v2 = { glmv2.x, glmv2.y, glmv2.z };

	return Bounds(v0, v1, v2);
}

cl_float3 BVH::GetCentroid(const Bounds& triangleBounds) const
{
	cl_float3 pMin = triangleBounds.pMin;
	cl_float3 pMax = triangleBounds.pMax;
	cl_float3 centroid;

	centroid.x = (pMin.x + pMax.x) * 0.5;
	centroid.y = (pMin.y + pMax.y) * 0.5;
	centroid.z = (pMin.z + pMax.z) * 0.5;

	return centroid;
}

/********** OCTREE NODE **********/

BVH::OctreeNode::OctreeNode(const Bounds& bounds)
	: IsLeaf(true), NodeBounds(bounds)
{
	for (uint8_t i = 0; i < 8; ++i)
		Children[i] = NULL;
}

BVH::OctreeNode::~OctreeNode()
{
	for (uint8_t i = 0; i < 8; ++i)
		if (Children[i] != NULL) delete Children[i];
}

/********** OCTREE **********/

BVH::Octree::Octree(BVH* bvh, cl_uint maxDepth)
	: m_Root(new OctreeNode(bvh->GetSceneBounds())), m_BVH(bvh), m_Bounds(bvh->GetSceneBounds()), m_MaxDepth(maxDepth)
{
}

BVH::Octree::~Octree()
{
	delete m_Root;
}

void BVH::Octree::Insert(cl_uint triangle)
{
	Insert(m_Root, triangle, m_Bounds, 1);
}

void BVH::Octree::Build()
{
}

Bounds BVH::Octree::GetChildBounds(cl_char index, const cl_float3& nodeCentroid, const Bounds& bounds) const
{
	Bounds childBounds;

	childBounds.pMin.x = (index & 4) ? nodeCentroid.x : bounds.pMin.x;
	childBounds.pMax.x = (index & 4) ? bounds.pMax.x : nodeCentroid.x;
	childBounds.pMin.y = (index & 2) ? nodeCentroid.y : bounds.pMin.y;
	childBounds.pMax.y = (index & 2) ? bounds.pMax.y : nodeCentroid.y;
	childBounds.pMin.z = (index & 1) ? nodeCentroid.z : bounds.pMin.z;
	childBounds.pMax.z = (index & 1) ? bounds.pMax.z : nodeCentroid.z;

	return childBounds;
}

Bounds BVH::Octree::GetTriangleBounds(cl_uint index) const
{
	Triangle triangle = m_BVH->m_Triangles[index];
	glm::mat4 transform = m_BVH->m_Transforms[triangle.Transform];

	glm::vec3 glmv0 = { m_BVH->m_Vertices[triangle.v0].x, m_BVH->m_Vertices[triangle.v0].y, m_BVH->m_Vertices[triangle.v0].z };
	glm::vec3 glmv1 = { m_BVH->m_Vertices[triangle.v1].x, m_BVH->m_Vertices[triangle.v1].y, m_BVH->m_Vertices[triangle.v1].z };
	glm::vec3 glmv2 = { m_BVH->m_Vertices[triangle.v2].x, m_BVH->m_Vertices[triangle.v2].y, m_BVH->m_Vertices[triangle.v2].z };

	glmv0 = transform * glm::vec4(glmv0, 1.0f);
	glmv1 = transform * glm::vec4(glmv1, 1.0f);
	glmv2 = transform * glm::vec4(glmv2, 1.0f);

	cl_float3 v0 = { glmv0.x, glmv0.y, glmv0.z };
	cl_float3 v1 = { glmv1.x, glmv1.y, glmv1.z };
	cl_float3 v2 = { glmv2.x, glmv2.y, glmv2.z };

	return Bounds(v0, v1, v2);
}

cl_float3 BVH::Octree::GetCentroid(const Bounds& triangleBounds) const
{
	cl_float3 pMin = triangleBounds.pMin;
	cl_float3 pMax = triangleBounds.pMax;
	cl_float3 centroid;

	centroid.x = (pMin.x + pMax.x) * 0.5;
	centroid.y = (pMin.y + pMax.y) * 0.5;
	centroid.z = (pMin.z + pMax.z) * 0.5;

	return centroid;
}

void BVH::Octree::Insert(BVH::OctreeNode* node, cl_uint triangle, const Bounds& bounds, uint32_t depth)
{
	// if node is leaf
	if (node->IsLeaf)
	{
		// if node is leaf and does not contain a triangle, or if maximum depth is reached
		if (!node->Triangles.size() || depth == m_MaxDepth)
			node->Triangles.push_back(triangle);
		// if node is leaf and contains a triangle
		else
		{
			node->IsLeaf = false;
			// insert existing triangles and new triangle
			while (node->Triangles.size())
			{
				Insert(node, node->Triangles.back(), bounds, depth);
				node->Triangles.pop_back();
			}
			Insert(node, triangle, bounds, depth);
		}
	}
	// if node is interior
	else
	{
		// Compute world bounds and centroid for triangle
		Bounds triangleBounds = GetTriangleBounds(triangle);
		cl_float3 triangleCentroid = GetCentroid(triangleBounds);

		// Compute centroid of node
		cl_float3 nodeCentroid = GetCentroid(node->NodeBounds);

		// Find octree node child index
		cl_char child = 0;
		if (triangleCentroid.x > nodeCentroid.x) child += 4;
		if (triangleCentroid.y > nodeCentroid.y) child += 2;
		if (triangleCentroid.z > nodeCentroid.z) child += 1;

		// Compute child bounds
		Bounds childBounds = GetChildBounds(child, nodeCentroid, bounds);
		
		// Create child node and insert triangle
		if (node->Children[child] == NULL)
		{
			node->Children[child] = new OctreeNode(childBounds);
			node->Children[child]->Depth = depth;
		}
		Insert(node->Children[child], triangle, childBounds, depth + 1);
	}
}