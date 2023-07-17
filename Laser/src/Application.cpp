#include "Application.h"

#include <iostream>

#include "Vertex.h"
#include "ModelLoader.h"
#include "Transform.h"
#include "BVH.h"

#define VERIFY(x) if (!x) return false

Application::Application()
	: m_GlobalWorkSize(0), m_LocalWorkSize(64),	m_NTriangles(0),
	m_Image(600, 600, 128, 128, Image::Format::ppm), m_Camera(45.0f, m_Image.GetProps().AspectRatio)
{
	m_GlobalWorkSize = m_Image.GetProps().TileHeight * m_Image.GetProps().TileWidth;
	m_AppStart = clock();
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
	m_Materials.resize(5);
	m_Materials[0] = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, false, false, 0.0f }; // white
	m_Materials[1] = { {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, false, false, 0.0f }; // red
	m_Materials[2] = { {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, false, false, 0.0f }; // green
	m_Materials[3] = { {1.0f, 1.0f, 1.0f}, {5.0f, 5.0f, 5.0f}, false, false, 0.0f }; // light
	//m_Materials[4] = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, true, false, 0.0f }; // metal/mirror
	//m_Materials[4] = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, false, true, 1.5f }; // glass
	m_Materials[4] = { {0.2f, 0.4f, 0.8f}, {0.0f, 0.0f, 0.0f}, false, false, 0.0f }; // blue matte

	// Set transforms
	Transform t;
	m_Transforms.resize(2);
	m_Transforms[0] = t.Generate(); // identity (index 0 reserved for when no transform is supplied)
	m_Transforms[1] = t.Generate(glm::vec3(0.5f, 0.5f, 1.0f), 45.0f, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.04f));

	// Construct BVH
	m_BVH = BVH(*m_Meshes[0].GetVerticesPtr(), *m_Meshes[0].GetTrianglesPtr(), m_Transforms);

	return true;
}

bool Application::GenBuffers()
{
	VERIFY(m_OCL.AddBuffer("output", CL_MEM_WRITE_ONLY, m_GlobalWorkSize * sizeof(cl_float3)));
	VERIFY(m_OCL.AddBuffer("imageProps", CL_MEM_READ_ONLY, sizeof(Image::Props)));
	VERIFY(m_OCL.AddBuffer("cameraProps", CL_MEM_READ_ONLY, sizeof(Camera::Props)));
	VERIFY(m_OCL.AddBuffer("vertices", CL_MEM_READ_ONLY, m_BVH.m_Vertices.size() * sizeof(Vertex)));
	VERIFY(m_OCL.AddBuffer("triangles", CL_MEM_READ_ONLY, m_BVH.m_Triangles.size() * sizeof(Triangle)));
	VERIFY(m_OCL.AddBuffer("materials", CL_MEM_READ_ONLY, m_Materials.size() * sizeof(Material)));
	VERIFY(m_OCL.AddBuffer("transforms", CL_MEM_READ_ONLY, m_BVH.m_Transforms.size() * sizeof(glm::mat4)));
	VERIFY(m_OCL.AddBuffer("bvh", CL_MEM_READ_ONLY, m_BVH.m_BVHLinearNodes.size() * sizeof(BVH::BVHLinearNode)));
	VERIFY(m_OCL.AddBuffer("stats", CL_MEM_READ_WRITE, sizeof(m_RenderStats)));

	return true;
}

bool Application::SetKernelArgs()
{
	VERIFY(m_OCL.SetKernelArg(0, "output"));
	VERIFY(m_OCL.SetKernelArg(1, "imageProps"));
	VERIFY(m_OCL.SetKernelArg(2, "cameraProps"));
	VERIFY(m_OCL.SetKernelArg(3, "vertices"));
	VERIFY(m_OCL.SetKernelArg(4, "triangles"));
	VERIFY(m_OCL.SetKernelArg(5, "materials"));
	VERIFY(m_OCL.SetKernelArg(6, "transforms"));
	VERIFY(m_OCL.SetKernelArg(7, "bvh"));
	VERIFY(m_OCL.SetKernelArg(8, "stats"));

	return true;
}

bool Application::Render()
{
	// TODO: Create profiler class and fix timing
	// clock_t timeStart = clock();
	m_RenderStart = clock();

	Image::Props imageProps = m_Image.GetProps();
	Camera::Props cameraProps = m_Camera.GetProps();

	// Write scene data to OpenCL buffers
	VERIFY(m_OCL.QueueWrite("imageProps", CL_TRUE, 0, sizeof(Image::Props), &imageProps));
	VERIFY(m_OCL.QueueWrite("cameraProps", CL_TRUE, 0, sizeof(Camera::Props), &cameraProps));
	VERIFY(m_OCL.QueueWrite("vertices", CL_TRUE, 0, m_BVH.m_Vertices.size() * sizeof(Vertex), m_BVH.m_Vertices.data()));
	VERIFY(m_OCL.QueueWrite("triangles", CL_TRUE, 0, m_BVH.m_Triangles.size() * sizeof(Triangle), m_BVH.m_Triangles.data()));
	VERIFY(m_OCL.QueueWrite("materials", CL_TRUE, 0, m_Materials.size() * sizeof(Material), m_Materials.data()));
	VERIFY(m_OCL.QueueWrite("transforms", CL_TRUE, 0, m_BVH.m_Transforms.size() * sizeof(glm::mat4), m_BVH.m_Transforms.data()));
	VERIFY(m_OCL.QueueWrite("bvh", CL_TRUE, 0, m_BVH.m_BVHLinearNodes.size() * sizeof(BVH::BVHLinearNode), m_BVH.m_BVHLinearNodes.data()));

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
		VERIFY(m_OCL.SetKernelArg(9, xOffset));
		VERIFY(m_OCL.SetKernelArg(10, yOffset));

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
		std::cout << "Done tile " << k + 1 << " of " << props.nColumns * props.nRows << std::endl;
	}

	// clock_t timeEnd = clock();
	// stats.RenderTime = (cl_float)(timeEnd - timeStart) / CLOCKS_PER_SEC;
	m_RenderEnd = clock();

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
	
	m_AppEnd = clock();
	std::cout << "App time: " << (float)(m_AppEnd - m_AppStart) / CLOCKS_PER_SEC << "s." << std::endl;
	std::cout << "Render time: " << (float)(m_RenderEnd - m_RenderStart) / CLOCKS_PER_SEC << "s." << std::endl;

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
