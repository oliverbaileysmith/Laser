#pragma once

#include <vector>

#include <CL/cl.hpp>

class Image
{
public:
	enum class Format
	{
		ppm = 0
	};

	Image(cl_uint width, cl_uint height, Format format);

	void WriteToFile() const;

public:
	Format m_Format;
	const cl_uint m_Width;
	const cl_uint m_Height;
	const cl_float m_AspectRatio;
	std::vector<cl_float3> m_Pixels;
};