#include "Image.h"

#include <iostream>
#include <fstream>

Image::Image(cl_uint width, cl_uint height, Format format)
	: m_Width(width), m_Height(height), m_Format(format),
		m_AspectRatio((cl_float)m_Width/(cl_float)m_Height)
{
	m_Pixels.reserve(m_Height / 64);
}

bool Image::WriteToFile(const std::string& filepath, cl_uint rowsPerExec) const
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
			int counter = 0;
			for (int j = 0; j < (float)m_Height / (float)rowsPerExec; j++)
			{
				for (int i = 0; i < m_Width * rowsPerExec; i++)
				{
					if (counter == m_Width * m_Height) break;
					fprintf(outputFile, "%d %d %d ",
						(cl_int)(clamp(m_Pixels[j][i].x) * 255),
						(cl_int)(clamp(m_Pixels[j][i].y) * 255),
						(cl_int)(clamp(m_Pixels[j][i].z) * 255));
					counter++;
				}
			}
	}
	
	fclose(outputFile);
	std::cout << "Finished writing to file." << std::endl;
	return true;
}