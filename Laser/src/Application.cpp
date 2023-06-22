#include "Application.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "ModelLoader.h"

#define VERIFY(x) if (!x) return false

Application::Application()
	: m_GlobalWorkSize(0), m_LocalWorkSize(64),	m_NTriangles(0),
	m_Image(600, 600, 128, 128, Image::Format::ppm), m_ViewportHeight(2.0f),
	m_ViewportWidth(m_ViewportHeight * m_Image.GetProps().AspectRatio),
	m_FocalLength(1.0f), m_CameraOrigin({ 0.0f,0.0f,1.0f }),
	m_UpperLeftCorner(m_CameraOrigin)
{
	m_GlobalWorkSize = m_Image.GetProps().TileHeight * m_Image.GetProps().TileWidth;

	// TODO: camera abstraction
	m_UpperLeftCorner.x -= m_ViewportWidth / 2.0f;
	m_UpperLeftCorner.y += m_ViewportHeight / 2.0f;
	m_UpperLeftCorner.z -= m_FocalLength;
}

bool Application::Init()
{
	// Initialize OpenCL
	VERIFY(m_OCL.Init());
	VERIFY(m_OCL.LoadKernel("cl/Laser.cl", "Laser"));

	// Set image tile rows and columns
	cl_uint nRows = 0;
	cl_uint nColumns = 0;
	m_Image.CalcTileRowsAndColumns(nRows, nColumns);
	m_Image.SetTileRowsAndColumns(nRows, nColumns);

	// Load geometry
	VERIFY(LoadModel("res/models/utah-teapot.obj"));

	// Set materials
	m_Materials.resize(4);
	m_Materials[0] = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} }; // white
	m_Materials[1] = { {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // red
	m_Materials[2] = { {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // green
	m_Materials[3] = { {0.0f, 0.0f, 0.0f}, {5.0f, 5.0f, 5.0f} }; // light

	m_Transforms.resize(2);
	m_Transforms[0] = glm::mat4(1.0f); // identity (index 0 reserved for when no transform is supplied)
	m_Transforms[1] = glm::scale(glm::mat4(1.0f), glm::vec3(0.4f));
	m_Transforms[1] = glm::rotate(m_Transforms[1], glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_Transforms[1] = glm::translate(m_Transforms[1], glm::vec3(0.0f, -1.8f, -1.5f));

	return true;
}

bool Application::GenBuffers()
{
	VERIFY(m_OCL.AddBuffer("output", CL_MEM_WRITE_ONLY, m_GlobalWorkSize * sizeof(cl_float3)));
	VERIFY(m_OCL.AddBuffer("vertices", CL_MEM_READ_ONLY, m_Meshes[0].GetVerticesPtr()->size() * sizeof(cl_float3)));
	VERIFY(m_OCL.AddBuffer("triangles", CL_MEM_READ_ONLY, m_Meshes[0].GetTrianglesPtr()->size() * sizeof(Triangle)));
	VERIFY(m_OCL.AddBuffer("materials", CL_MEM_READ_ONLY, m_Materials.size() * sizeof(Material)));
	VERIFY(m_OCL.AddBuffer("transforms", CL_MEM_READ_ONLY, m_Transforms.size() * sizeof(glm::mat4)));
	VERIFY(m_OCL.AddBuffer("stats", CL_MEM_READ_WRITE, sizeof(m_RenderStats)));

	return true;
}

bool Application::SetKernelArgs()
{
	Image::Props props = m_Image.GetProps();

	VERIFY(m_OCL.SetKernelArg(0, "output"));
	VERIFY(m_OCL.SetKernelArg(1, props.Width));
	VERIFY(m_OCL.SetKernelArg(2, props.Height));
	VERIFY(m_OCL.SetKernelArg(3, props.AspectRatio));
	VERIFY(m_OCL.SetKernelArg(4, m_ViewportWidth));
	VERIFY(m_OCL.SetKernelArg(5, m_ViewportHeight));
	VERIFY(m_OCL.SetKernelArg(6, m_FocalLength));
	VERIFY(m_OCL.SetKernelArg(7, m_CameraOrigin));
	VERIFY(m_OCL.SetKernelArg(8, m_UpperLeftCorner));
	VERIFY(m_OCL.SetKernelArg(9, "vertices"));
	VERIFY(m_OCL.SetKernelArg(10, "triangles"));
	VERIFY(m_OCL.SetKernelArg(11, m_NTriangles));
	VERIFY(m_OCL.SetKernelArg(12, "materials"));
	VERIFY(m_OCL.SetKernelArg(13, "transforms"));
	VERIFY(m_OCL.SetKernelArg(14, "stats"));
	VERIFY(m_OCL.SetKernelArg(17, props.TileWidth));
	VERIFY(m_OCL.SetKernelArg(18, props.TileHeight));

	return true;
}

bool Application::Render()
{
	// TODO: Create profiler class and fix timing
	// clock_t timeStart = clock();

	// Write scene data to OpenCL buffers
	VERIFY(m_OCL.QueueWrite("vertices", CL_TRUE, 0, m_Meshes[0].GetVerticesPtr()->size() * sizeof(cl_float3), m_Meshes[0].GetVerticesPtr()->data()));
	VERIFY(m_OCL.QueueWrite("triangles", CL_TRUE, 0, m_Meshes[0].GetTrianglesPtr()->size() * sizeof(Triangle), m_Meshes[0].GetTrianglesPtr()->data()));
	VERIFY(m_OCL.QueueWrite("materials", CL_TRUE, 0, m_Materials.size() * sizeof(Material), m_Materials.data()));
	VERIFY(m_OCL.QueueWrite("transforms", CL_TRUE, 0, m_Transforms.size() * sizeof(glm::mat4), m_Transforms.data()));

	Image::Props props = m_Image.GetProps();
	// Execute kernel for each tile
	for (int k = 0; k < props.nRows * props.nColumns; k++)
	{
		// Create tile
		Image::Tile tile(props.TileWidth, props.TileHeight);

		// Calculate current tile offsets
		cl_uint tileX = k % props.nRows;
		cl_uint tileY = k / props.nRows;

		cl_uint xOffset = tileX * props.TileWidth;
		cl_uint yOffset = tileY * props.TileHeight;

		// Send per-tile offsets to OpenCL device
		VERIFY(m_OCL.SetKernelArg(15, xOffset));
		VERIFY(m_OCL.SetKernelArg(16, yOffset));

		// Execute kernel
		VERIFY(m_OCL.QueueKernel(NULL, m_GlobalWorkSize, m_LocalWorkSize));
		
		// Read result to current tile
		VERIFY(m_OCL.QueueRead("output", CL_TRUE, 0, m_GlobalWorkSize * sizeof(cl_float3), tile.Pixels.data()));

		// Merge tile into image
		for (int j = 0; j < props.TileHeight; j++)
		{
			int y = yOffset + j;
			if (y == props.Height) break;
			for (int i = 0; i < props.TileWidth; i++)
			{
				int x = xOffset + i;
				if (x == props.Width) break;
				m_Image.m_Pixels[y][x] = tile.Pixels[j * props.TileWidth + i];
			}
		}
	}

	// clock_t timeEnd = clock();
	// stats.RenderTime = (cl_float)(timeEnd - timeStart) / CLOCKS_PER_SEC;

	// Read profiler stats from OpenCL device
	VERIFY(m_OCL.QueueRead("stats", CL_TRUE, 0, sizeof(m_RenderStats), &m_RenderStats));

	return true;
}

bool Application::WriteOutput()
{
	// TODO: write render stats here
	// Write render stats to console
	/*std::cout << "Render time:                " << stats.RenderTime << " seconds" << std::endl;
	std::cout << "Primary rays:               " << stats.n_PrimaryRays << std::endl;
	std::cout << "Ray-triangle tests:         " << stats.n_RayTriangleTests << std::endl;
	std::cout << "Ray-triangle intersections: " << stats.n_RayTriangleIsects << std::endl << std::endl;*/

	// Write image to file
	VERIFY(m_Image.WriteToFile("output.ppm"));
	
	return true;
}

bool Application::LoadModel(const std::string& filepath)
{
	ModelLoader loader;
	m_Meshes = loader.LoadModel(filepath);

	// Ensure model was loaded
	if (m_Meshes.empty())
	{
		std::cout << "No valid models were loaded, terminating program!" << std::endl;
		return false;
	}

	m_NTriangles = m_Meshes[0].GetTrianglesPtr()->size();
	
	return true;
}
