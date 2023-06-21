#include "Image.h"

#include <iostream>
#include <fstream>

Image::Image(cl_uint width, cl_uint height, cl_uint tileWidth, cl_uint tileHeight, Format format)
	: m_Width(width), m_Height(height), m_TileWidth(tileWidth), m_TileHeight(tileHeight),
		m_Format(format), m_AspectRatio((cl_float)m_Width/(cl_float)m_Height)
{
	m_Pixels.resize(m_Height);
	for (int i = 0; i < m_Height; i++)
		m_Pixels[i].resize(m_Width);
}

bool Image::WriteToFile(const std::string& filepath) const
{
	std::cout << "Writing to file \"" << filepath << "\"..." << std::endl;

	// Open file and ensure it was successfully created
	FILE* outputFile = nullptr;
	fopen_s(&outputFile, filepath.c_str(), "w");
	if (!outputFile)
	{
		std::cout << "Failed to open file " << filepath << "." << std::endl;
		return false;
	}

	switch (m_Format)
	{
		case Format::ppm:
		default:
			fprintf(outputFile, "P3\n%d %d\n%d\n", m_Width, m_Height, 255);

			// Convert each pixel's RGB values from [0.0f, 1.0f] to [0, 255] and write to file
			for (int j = 0; j < m_Height; j++)
			{
				for (int i = 0; i < m_Width; i++)
				{
					fprintf(outputFile, "%d %d %d ",
						(cl_int)(clamp(m_Pixels[j][i].x) * 255),
						(cl_int)(clamp(m_Pixels[j][i].y) * 255),
						(cl_int)(clamp(m_Pixels[j][i].z) * 255));
				}
			}
	}
	
	fclose(outputFile);
	std::cout << "Finished writing to file." << std::endl;
	return true;
}