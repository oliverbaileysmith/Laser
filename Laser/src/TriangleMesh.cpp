#include "TriangleMesh.h"

TriangleMesh::TriangleMesh(std::vector<Vertex> vertices,
	std::vector<Triangle> triangles)
	: m_Vertices(vertices), m_Triangles(triangles)
{
}
