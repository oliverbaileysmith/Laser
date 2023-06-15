#include "TriangleMesh.h"

TriangleMesh::TriangleMesh()
{
    // dummy mesh data

    m_Vertices.resize(12);
    m_Vertices[0] =  { -0.5f, -0.500f,  0.2f }; // front bottom left
    m_Vertices[1] =  { -0.5f, -0.500f, -0.8f }; // back bottom left
    m_Vertices[2] =  { -0.5f,  0.500f,  0.2f }; // front top left
    m_Vertices[3] =  { -0.5f,  0.500f, -0.8f }; // back top left
    m_Vertices[4] =  {  0.5f, -0.500f,  0.2f }; // front bottom right
    m_Vertices[5] =  {  0.5f, -0.500f, -0.8f }; // back bottom right
    m_Vertices[6] =  {  0.5f,  0.500f,  0.2f }; // front top right
    m_Vertices[7] =  {  0.5f,  0.500f, -0.8f }; // back top right
    m_Vertices[8] =  { -0.3f,  0.499f, -0.6f }; // light back left
    m_Vertices[9] =  {  0.3f,  0.499f, -0.6f }; // light back right
    m_Vertices[10] = {  0.3f,  0.499f,  0.0f }; // light front right
    m_Vertices[11] = { -0.3f,  0.499f,  0.0f }; // light front left

    m_Triangles.resize(14);
    m_Triangles[0] =  { 0,  1,  3, 1 }; // left
    m_Triangles[1] =  { 0,  3,  2, 1 };
    m_Triangles[2] =  { 1,  5,  7, 0 }; // back
    m_Triangles[3] =  { 1,  7,  3, 0 };
    m_Triangles[4] =  { 5,  4,  6, 2 }; // right
    m_Triangles[5] =  { 5,  6,  7, 2 };
    m_Triangles[6] =  { 4,  0,  2, 0 }; // front
    m_Triangles[7] =  { 4,  2,  6, 0 };
    m_Triangles[8] =  { 3,  7,  6, 0 }; // top
    m_Triangles[9] =  { 3,  6,  2, 0 };
    m_Triangles[10] = { 0,  4,  5, 0 }; // bottom
    m_Triangles[11] = { 0,  5,  1, 0 };
    m_Triangles[12] = { 8,  9, 10, 3 }; // light
    m_Triangles[13] = { 8, 10, 11, 3 };
}

const std::vector<cl_float3>* const TriangleMesh::GetVerticesPtr() const
{
    return &m_Vertices;
}

const std::vector<Triangle>* const TriangleMesh::GetTrianglesPtr() const
{
    return &m_Triangles;
}