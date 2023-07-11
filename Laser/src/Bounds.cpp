#include <algorithm>

#include "Bounds.h"

Bounds::Bounds()
    : pMin({ INFINITY, INFINITY, INFINITY }), pMax({ -INFINITY, -INFINITY, -INFINITY })
{
}

Bounds::Bounds(const cl_float3& v0, const cl_float3& v1, const cl_float3& v2)
{
    Bounds b(v0, v1);
    b.Extend(v2);
    pMin = b.pMin;
    pMax = b.pMax;
}

Bounds::Bounds(const cl_float3& p0, const cl_float3& p1)
{
    pMin.x = std::min(p0.x, p1.x);
    pMin.y = std::min(p0.y, p1.y);
    pMin.z = std::min(p0.z, p1.z);

    pMax.x = std::max(p0.x, p1.x);
    pMax.y = std::max(p0.y, p1.y);
    pMax.z = std::max(p0.z, p1.z);
}

void Bounds::Extend(const cl_float3& p)
{
    pMin.x = std::min(pMin.x, p.x);
    pMin.y = std::min(pMin.y, p.y);
    pMin.z = std::min(pMin.z, p.z);

    pMax.x = std::max(pMax.x, p.x);
    pMax.y = std::max(pMax.y, p.y);
    pMax.z = std::max(pMax.z, p.z);
}

void Bounds::Join(const Bounds& b)
{
    pMin.x = std::min(pMin.x, b.pMin.x);
    pMin.y = std::min(pMin.y, b.pMin.y);
    pMin.z = std::min(pMin.z, b.pMin.z);

    pMax.x = std::max(pMax.x, b.pMax.x);
    pMax.y = std::max(pMax.y, b.pMax.y);
    pMax.z = std::max(pMax.z, b.pMax.z);
}

cl_uint Bounds::GetLargestDimension() const
{
    cl_float diagonalX = pMax.x - pMin.x;
    cl_float diagonalY = pMax.y - pMin.y;
    cl_float diagonalZ = pMax.z - pMin.z;

    if (diagonalX > diagonalY && diagonalX > diagonalZ)
        return 0;
    else if (diagonalY > diagonalZ)
        return 1;
    else
        return 2;
}
