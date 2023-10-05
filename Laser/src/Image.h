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

	struct Props
	{
		cl_uint Width;
		cl_uint Height;
		cl_float AspectRatio;
		cl_uint TileWidth;
		cl_uint TileHeight;
		cl_uint nRows;
		cl_uint nColumns;
		Format Format;
	};

	struct Tile
	{
		Tile(cl_uint width, cl_uint height) : Width(width), Height(height)
		{
			// Create empty tile of pixels
			Pixels.resize(Height * Width);
		}
		cl_uint Width;
		cl_uint Height;
		std::vector<cl_float3> Pixels;
	};

	Image(cl_uint width, cl_uint height, cl_uint tileWidth, cl_uint tileHeight,
		Format format);

	bool WriteToFile(const std::string &filepath) const;

	void CalcTileRowsAndColumns(cl_uint &nRows, cl_uint &nColumns) const;
	void SetTileRowsAndColumns(cl_uint nRows, cl_uint nColumns);

	Props GetProps() const;

	std::vector<std::vector<cl_float3>> m_Pixels;

private:
	inline cl_float clamp(cl_float x) const
	{
		return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x;
	}

	Props m_Props;
};
