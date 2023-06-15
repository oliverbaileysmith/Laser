#include "Image.h"

#include <iostream>
#include <fstream>

Image::Image(cl_uint width, cl_uint height, Format format)
	: m_Width(width), m_Height(height), m_Format(format),
		m_AspectRatio((cl_float)m_Width/(cl_float)m_Height)
{
	m_Pixels.resize(m_Width * m_Height);
}

void Image::WriteToFile(std::string filepath) const
{
	switch (m_Format)
	{
		case Format::ppm:
		default:
			std::cout << std::endl << "Writing to file \"" << filepath << "\"..." << std::endl;
			std::ofstream outputFile;
			outputFile.open(filepath);

			outputFile << "P3" << std::endl;
			outputFile << m_Width << " " << m_Height << std::endl;
			outputFile << "255" << std::endl;

			for (int i = 0; i < m_Width * m_Height; i++)
			{
				outputFile
					<< (int32_t)(m_Pixels[i].x * 255) << " "
					<< (int32_t)(m_Pixels[i].y * 255) << " "
					<< (int32_t)(m_Pixels[i].z * 255) << std::endl;
			}
			outputFile.close();
	}
	std::cout << "Finished writing to file." << std::endl;
}

const cl_float3* Image::GetPixelsPtr() const
{
	return m_Pixels.data();
}
