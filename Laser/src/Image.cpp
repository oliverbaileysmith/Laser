#include "Image.h"

#include <iostream>
#include <fstream>

Image::Image(cl_uint width, cl_uint height, cl_uint tileWidth, cl_uint tileHeight, Format format)
{
	m_Props.Width = width;
	m_Props.Height = height;
	m_Props.TileWidth = tileWidth;
	m_Props.TileHeight = tileHeight;
	m_Props.AspectRatio = (float)width / (float)height;
	m_Props.nRows = 0;
	m_Props.nColumns = 0;
	m_Props.Format = format;

	m_Pixels.resize(m_Props.Height);
	for (int i = 0; i < m_Props.Height; i++)
		m_Pixels[i].resize(m_Props.Width);
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

	switch (m_Props.Format)
	{
		case Format::ppm:
		default:
			fprintf(outputFile, "P3\n%d %d\n%d\n", m_Props.Width, m_Props.Height, 255);

			// Convert each pixel's RGB values from [0.0f, 1.0f] to [0, 255] and write to file
			for (int j = 0; j < m_Props.Height; j++)
			{
				for (int i = 0; i < m_Props.Width; i++)
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

void Image::CalcTileRowsAndColumns(cl_uint& nRows, cl_uint& nColumns) const
{
	nRows = m_Props.Width / m_Props.TileWidth;
	nColumns = m_Props.Height / m_Props.TileHeight;
	if (m_Props.Width % m_Props.TileWidth != 0)
		nRows++;
	if (m_Props.Height % m_Props.TileHeight != 0)
		nColumns++;
}

void Image::SetTileRowsAndColumns(cl_uint nRows, cl_uint nColumns)
{
	m_Props.nRows = nRows;
	m_Props.nColumns = nColumns;
}

Image::Props Image::GetProps() const
{
	return m_Props;
}
