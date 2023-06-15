#include "Image.h"

Image::Image(cl_uint width, cl_uint height, Format format)
	: m_Width(width), m_Height(height), m_Format(format),
		m_AspectRatio((cl_float)m_Width/(cl_float)m_Height)
{
	m_Pixels.resize(m_Width * m_Height);
}

void Image::WriteToFile() const
{
}
