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

	void WriteToFile(std::string filepath) const;
	const cl_float3* GetPixelsPtr() const;

private:
	inline cl_float clamp(cl_float x) const { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

	Format m_Format;
	const cl_uint m_Width;
	const cl_uint m_Height;
	const cl_float m_AspectRatio;
	std::vector<cl_float3> m_Pixels;
};