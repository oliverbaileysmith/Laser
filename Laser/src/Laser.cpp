#include <iostream>
#include <fstream>
#include <vector>

#include <CL/cl.hpp>

#include "Triangle.h"
#include "TriangleMesh.h"
#include "ModelLoader.h"
#include "Material.h"
#include "RenderStats.h"
#include "Image.h"

int main()
{
	// OpenCL platform
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	if (platforms.empty())
	{
		std::cout << "No OpenCL platform available." << std::endl;
		return -1;
	}

	cl::Platform platform = platforms[0];

	// OpenCL device
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

	if (devices.empty())
	{
		std::cout << "No OpenCL GPU device available." << std::endl;
		return -1;
	}

	cl::Device device = devices[0];

	// Print OpenCL platform and device info
	std::cout << "OpenCL platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
	std::cout << "OpenCL device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
	std::cout << "OpenCL version: " << device.getInfo<CL_DEVICE_VERSION>() << std::endl;

	// OpenCL context
	cl::Context context(device);

	// OpenCL kernel source
	std::string kernelSrc;
	std::string line;
	std::ifstream kernelFile("cl/Laser.cl");

	while (std::getline(kernelFile, line))
	{
		kernelSrc += line + '\n';
	}

	kernelFile.close();

	// OpenCL program
	cl::Program program(context, kernelSrc.c_str());

	cl_int buildError = program.build({ device }, "-I cl");
	if (buildError)
	{
		std::cout << std::endl << "OpenCL program compilation error: " << buildError << std::endl;
		std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << "Build log:" << std::endl << buildLog << std::endl;
		return -1;
	}

	// OpenCL kernel
	cl::Kernel kernel(program, "Laser");

	// Host data
	const cl_int imageWidth = 600;
	const cl_int imageHeight = 600;
	const cl_float aspectRatio = (cl_float)imageWidth / (cl_float)imageHeight;
	Image image(imageWidth, imageHeight, Image::Format::ppm);

	cl_float viewportHeight = 2.0f;
	cl_float viewportWidth = viewportHeight * aspectRatio;
	cl_float focalLength = 1.0f;
	cl_float3 cameraOrigin = { 0.0f,0.0f,1.0f };
	cl_float3 upperLeftCorner = cameraOrigin;
	upperLeftCorner.x -= viewportWidth / 2.0f;
	upperLeftCorner.y += viewportHeight / 2.0f;
	upperLeftCorner.z -= focalLength;

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
	cl::Buffer clOutput(context, CL_MEM_WRITE_ONLY, imageWidth * imageHeight * sizeof(cl_float3));
	cl::Buffer clStats(context, CL_MEM_READ_WRITE, sizeof(RenderStats));
	cl::Buffer clVertices(context, CL_MEM_READ_ONLY, meshes[0].GetVerticesPtr()->size() * sizeof(cl_float3));
	cl::Buffer clTriangles(context, CL_MEM_READ_ONLY, meshes[0].GetTrianglesPtr()->size() * sizeof(Triangle));
	cl::Buffer clMaterials(context, CL_MEM_READ_ONLY, sizeof(materials));

	// OpenCL kernel arguments
	kernel.setArg(0, clOutput);
	kernel.setArg(1, imageWidth);
	kernel.setArg(2, imageHeight);
	kernel.setArg(3, aspectRatio);
	kernel.setArg(4, viewportWidth);
	kernel.setArg(5, viewportHeight);
	kernel.setArg(6, focalLength);
	kernel.setArg(7, cameraOrigin);
	kernel.setArg(8, upperLeftCorner);
	kernel.setArg(9, clVertices);
	kernel.setArg(10, clTriangles);
	kernel.setArg(11, n_Triangles);
	kernel.setArg(12, clMaterials);
	kernel.setArg(13, clStats);

	// OpenCL command queue
	cl::CommandQueue queue(context, device);

	// OpenCL work items
	std::size_t globalWorkSize = imageWidth * imageHeight;
	std::size_t localWorkSize = 64;

	clock_t timeStart = clock();

	// Queue kernel execution and read result from device buffer
	queue.enqueueWriteBuffer(clVertices, CL_TRUE, 0, meshes[0].GetVerticesPtr()->size() * sizeof(cl_float3), meshes[0].GetVerticesPtr()->data());
	queue.enqueueWriteBuffer(clTriangles, CL_TRUE, 0, meshes[0].GetTrianglesPtr()->size() * sizeof(Triangle), meshes[0].GetTrianglesPtr()->data());
	queue.enqueueWriteBuffer(clMaterials, CL_TRUE, 0, sizeof(materials), &materials);
	queue.enqueueNDRangeKernel(kernel, NULL, globalWorkSize, localWorkSize);
	queue.enqueueReadBuffer(clOutput, CL_TRUE, 0, imageWidth * imageHeight * sizeof(cl_float3), (void*)image.GetPixelsPtr());
	queue.enqueueReadBuffer(clStats, CL_TRUE, 0, sizeof(RenderStats), &stats);

	clock_t timeEnd = clock();
	stats.RenderTime = (cl_float)(timeEnd - timeStart) / CLOCKS_PER_SEC;

	// Write render stats to console
	std::cout << std::endl;
	std::cout << "Render time:                " << stats.RenderTime << " seconds" << std::endl;
	std::cout << "Primary rays:               " << stats.n_PrimaryRays << std::endl;
	std::cout << "Ray-triangle tests:         " << stats.n_RayTriangleTests << std::endl;
	std::cout << "Ray-triangle intersections: " << stats.n_RayTriangleIsects << std::endl;

	image.WriteToFile("output.ppm");
}