#include <iostream>
#include <fstream>
#include <vector>

#include <CL/cl.hpp>

#include "OpenCLContext.h"
#include "Triangle.h"
#include "TriangleMesh.h"
#include "ModelLoader.h"
#include "Material.h"
#include "RenderStats.h"
#include "Image.h"

void CalcRowsAndColumns(cl_uint imageWidth, cl_uint imageHeight, cl_uint tileWidth, cl_uint tileHeight, cl_uint& nRows, cl_uint& nColumns)
{
	nRows = imageWidth / tileWidth;
	nColumns = imageHeight / tileHeight;
	if (imageWidth % tileWidth != 0)
		nRows++;
	if (imageHeight % tileHeight != 0)
		nColumns++;
}

int main()
{
	OpenCLContext ocl;

	if (!ocl.Init()) return -1;

	if (!ocl.LoadKernel("cl/Laser.cl", "Laser")) return -1;

	// Host data
	const cl_uint imageWidth = 600;
	const cl_uint imageHeight = 600;
	const cl_uint tileWidth = 192;
	const cl_uint tileHeight = 192;
	const cl_float aspectRatio = (cl_float)imageWidth / (cl_float)imageHeight;
	Image image(imageWidth, imageHeight, tileWidth, tileHeight, Image::Format::ppm);

	cl_float viewportHeight = 2.0f;
	cl_float viewportWidth = viewportHeight * aspectRatio;
	cl_float focalLength = 1.0f;
	cl_float3 cameraOrigin = { 0.0f,0.0f,1.0f };
	cl_float3 upperLeftCorner = cameraOrigin;
	upperLeftCorner.x -= viewportWidth / 2.0f;
	upperLeftCorner.y += viewportHeight / 2.0f;
	upperLeftCorner.z -= focalLength;

	cl_uint nRows;
	cl_uint nColumns;
	CalcRowsAndColumns(imageWidth, imageHeight, tileWidth, tileHeight, nRows, nColumns);

	RenderStats stats;

	ModelLoader loader;
	std::vector<TriangleMesh> meshes = loader.LoadModel("res/models/utah-teapot.obj");
	if (meshes.empty())
	{
		std::cout << "No valid models were loaded, terminating program!" << std::endl;
		return -1;
	}
	int n_Triangles = meshes[0].GetTrianglesPtr()->size();
	
	Material materials[4];
	materials[0] = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} }; // white
	materials[1] = { {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // red
	materials[2] = { {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // green
	materials[3] = { {0.0f, 0.0f, 0.0f}, {5.0f, 5.0f, 5.0f} }; // light

	// OpenCL device data
	if (!ocl.AddBuffer("output", CL_MEM_WRITE_ONLY, tileWidth * tileHeight * sizeof(cl_float3))) return -1;
	if (!ocl.AddBuffer("vertices", CL_MEM_READ_ONLY, meshes[0].GetVerticesPtr()->size() * sizeof(cl_float3))) return -1;
	if (!ocl.AddBuffer("triangles", CL_MEM_READ_ONLY, meshes[0].GetTrianglesPtr()->size() * sizeof(Triangle))) return -1;
	if (!ocl.AddBuffer("materials", CL_MEM_READ_ONLY, sizeof(materials))) return -1;
	if (!ocl.AddBuffer("stats", CL_MEM_READ_WRITE, sizeof(RenderStats))) return -1;

	// OpenCL kernel arguments
	if (!ocl.SetKernelArg(0, "output")) return -1;
	if (!ocl.SetKernelArg(1, imageWidth)) return -1;
	if (!ocl.SetKernelArg(2, imageHeight)) return -1;
	if (!ocl.SetKernelArg(3, aspectRatio)) return -1;
	if (!ocl.SetKernelArg(4, viewportWidth)) return -1;
	if (!ocl.SetKernelArg(5, viewportHeight)) return -1;
	if (!ocl.SetKernelArg(6, focalLength)) return -1;
	if (!ocl.SetKernelArg(7, cameraOrigin)) return -1;
	if (!ocl.SetKernelArg(8, upperLeftCorner)) return -1;
	if (!ocl.SetKernelArg(9, "vertices")) return -1;
	if (!ocl.SetKernelArg(10, "triangles")) return -1;
	if (!ocl.SetKernelArg(11, n_Triangles)) return -1;
	if (!ocl.SetKernelArg(12, "materials")) return -1;
	if (!ocl.SetKernelArg(13, "stats")) return -1;
	if (!ocl.SetKernelArg(16, tileWidth)) return -1;
	if (!ocl.SetKernelArg(17, tileHeight)) return -1;

	// OpenCL work items
	std::size_t globalWorkSize = tileWidth * tileHeight;
	std::size_t localWorkSize = 64;

	clock_t timeStart = clock();

	// Queue kernel execution and read result from device buffer
	if (!ocl.QueueWrite("vertices", CL_TRUE, 0, meshes[0].GetVerticesPtr()->size() * sizeof(cl_float3), meshes[0].GetVerticesPtr()->data())) return -1;
	if (!ocl.QueueWrite("triangles", CL_TRUE, 0, meshes[0].GetTrianglesPtr()->size() * sizeof(Triangle), meshes[0].GetTrianglesPtr()->data())) return -1;
	if (!ocl.QueueWrite("materials", CL_TRUE, 0, sizeof(materials), &materials)) return -1;
	
	for (int k = 0; k < nRows * nColumns; k++)
	{
		cl_uint tileX = k % nRows;
		cl_uint tileY = k / nRows;

		cl_uint xOffset = tileX * tileWidth;
		cl_uint yOffset = tileY * tileHeight;

		Image::Tile tile(tileWidth, tileHeight);
		if (!ocl.SetKernelArg(14, xOffset)) return -1;
		if (!ocl.SetKernelArg(15, yOffset)) return -1;

		if (!ocl.QueueKernel(NULL, globalWorkSize, localWorkSize)) return -1;
		if (!ocl.QueueRead("output", CL_TRUE, 0, globalWorkSize * sizeof(cl_float3), tile.Pixels.data())) return -1;

		for (int j = 0; j < tileHeight; j++)
		{
			int y = yOffset + j;
			if (y == imageHeight) break;
			for (int i = 0; i < tileWidth; i++)
			{
				int x = xOffset + i;
				if (x == imageWidth) break;
				image.m_Pixels[y][x] = tile.Pixels[j * tileWidth + i];
			}
		}
	}

	if (!ocl.QueueRead("stats", CL_TRUE, 0, sizeof(RenderStats), &stats)) return -1;

	clock_t timeEnd = clock();
	stats.RenderTime = (cl_float)(timeEnd - timeStart) / CLOCKS_PER_SEC;

	// Write render stats to console
	std::cout << "Render time:                " << stats.RenderTime << " seconds" << std::endl;
	std::cout << "Primary rays:               " << stats.n_PrimaryRays << std::endl;
	std::cout << "Ray-triangle tests:         " << stats.n_RayTriangleTests << std::endl;
	std::cout << "Ray-triangle intersections: " << stats.n_RayTriangleIsects << std::endl << std::endl;

	if (!image.WriteToFile("output.ppm")) return -1;
	return 0;
}