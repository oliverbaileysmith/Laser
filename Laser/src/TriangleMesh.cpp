#include "TriangleMesh.h"

TriangleMesh::TriangleMesh(std::vector<Vertex> vertices, std::vector<Triangle> triangles)
    : m_Vertices(vertices), m_Triangles(triangles)
{
    // TODO: Remove this
    // Add Cornell-style room and light for testing purposes
    m_Vertices.push_back({ -3.0f, -3.00f,  3.0f }); // front bottom left
    m_Vertices.push_back({ -3.0f, -3.00f, -3.0f }); // back bottom left
    m_Vertices.push_back({ -3.0f,  3.00f,  3.0f }); // front top left
    m_Vertices.push_back({ -3.0f,  3.00f, -3.0f }); // back top left
    m_Vertices.push_back({  3.0f, -3.00f,  3.0f }); // front bottom right
    m_Vertices.push_back({  3.0f, -3.00f, -3.0f }); // back bottom right
    m_Vertices.push_back({  3.0f,  3.00f,  3.0f }); // front top right
    m_Vertices.push_back({  3.0f,  3.00f, -3.0f }); // back top right
    m_Vertices.push_back({ -1.5f,  2.99f, -1.5f }); // light back left
    m_Vertices.push_back({  1.5f,  2.99f, -1.5f }); // light back right
    m_Vertices.push_back({  1.5f,  2.99f,  1.5f }); // light front right
    m_Vertices.push_back({ -1.5f,  2.99f,  1.5f }); // light front left

    cl_uint n = m_Vertices.size() - 12;
    m_Triangles.push_back({ n + 0,  n + 1,  n + 3, 1, 0 }); // left
    m_Triangles.push_back({ n + 0,  n + 3,  n + 2, 1, 0 });
    m_Triangles.push_back({ n + 1,  n + 5,  n + 7, 0, 0 }); // back
    m_Triangles.push_back({ n + 1,  n + 7,  n + 3, 0, 0 });
    m_Triangles.push_back({ n + 5,  n + 4,  n + 6, 2, 0 }); // right
    m_Triangles.push_back({ n + 5,  n + 6,  n + 7, 2, 0 });
    m_Triangles.push_back({ n + 4,  n + 0,  n + 2, 0, 0 }); // front
    m_Triangles.push_back({ n + 4,  n + 2,  n + 6, 0, 0 });
    m_Triangles.push_back({ n + 3,  n + 7,  n + 6, 0, 0 }); // top
    m_Triangles.push_back({ n + 3,  n + 6,  n + 2, 0, 0 });
    m_Triangles.push_back({ n + 0, n +  4, n +  5, 0, 0 }); // bottom
    m_Triangles.push_back({ n + 0, n +  5, n +  1, 0, 0 });
    m_Triangles.push_back({ n + 8, n +  9, n + 10, 3, 0 }); // light
    m_Triangles.push_back({ n + 8, n + 10, n + 11, 3, 0 });
}

const std::vector<Vertex>* const TriangleMesh::GetVerticesPtr() const
{
    return &m_Vertices;
}

const std::vector<Triangle>* const TriangleMesh::GetTrianglesPtr() const
{
    return &m_Triangles;
}