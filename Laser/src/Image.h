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

	bool WriteToFile(const std::string& filepath, cl_uint rowsPerExec) const;

	std::vector<std::vector<cl_float3>> m_Pixels;

private:
	inline cl_float clamp(cl_float x) const { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

	Format m_Format;
	const cl_uint m_Width;
	const cl_uint m_Height;
	const cl_float m_AspectRatio;
};