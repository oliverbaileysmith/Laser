#include <iostream>
#include <fstream>
#include <vector>

#include <CL/cl.hpp>

struct Triangle
{
	cl_float3 v0;
	cl_float3 v1;
	cl_float3 v2;
};

struct RenderStats
{
	cl_uint n_PrimaryRays = 0;
	cl_uint n_RayTriangleTests = 0;
	cl_uint n_RayTriangleIsects = 0;
	cl_float renderTime = 0.0f; // Time in seconds
};

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
	std::ifstream kernelFile("src/Laser.cl");

	while (std::getline(kernelFile, line))
	{
		kernelSrc += line + '\n';
	}

	kernelFile.close();

	// OpenCL program
	cl::Program program(context, kernelSrc.c_str());

	cl_int buildError = program.build({ device }, "");
	if (buildError)
	{
		std::cout << "OpenCL program compilation error: " << buildError << std::endl;
		return -1;
	}

	// OpenCL kernel
	cl::Kernel kernel(program, "Laser");

	// Host data
	const cl_int imageWidth = 1280;
	const cl_int imageHeight = 720;
	const cl_float aspectRatio = (cl_float)imageWidth / (cl_float)imageHeight;
	cl_float viewportHeight = 2.0f;
	cl_float viewportWidth = viewportHeight * aspectRatio;
	cl_float focalLength = 1.0f;
	cl_float3 cameraOrigin = { 0.0f,0.0f,0.0f };
	cl_float3 upperLeftCorner = cameraOrigin;
	upperLeftCorner.x -= viewportWidth / 2.0f;
	upperLeftCorner.y += viewportHeight / 2.0f;
	upperLeftCorner.z -= focalLength;

	cl_float3* cpuOutput = new cl_float3[imageWidth * imageHeight];

	Triangle tri;
	tri.v0 = { -0.5f, -0.5f, -1.0f };
	tri.v1 = {  0.5f, -0.5f, -1.0f };
	tri.v2 = {  0.0f,  0.5f, -1.0f };

	RenderStats stats;

	// OpenCL device data
	cl::Buffer clOutput(context, CL_MEM_WRITE_ONLY, imageWidth * imageHeight * sizeof(cl_float3));
	cl::Buffer clTriangle(context, CL_MEM_READ_ONLY, sizeof(tri));
	cl::Buffer clStats(context, CL_MEM_READ_WRITE, sizeof(RenderStats));

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
	kernel.setArg(9, clTriangle);
	kernel.setArg(10, clStats);

	// OpenCL command queue
	cl::CommandQueue queue(context, device);

	// OpenCL work items
	std::size_t globalWorkSize = imageWidth * imageHeight;
	std::size_t localWorkSize = 64;

	clock_t timeStart = clock();

	// Queue kernel execution and read result from device buffer
	queue.enqueueWriteBuffer(clTriangle, CL_TRUE, 0, sizeof(tri), &tri);
	queue.enqueueNDRangeKernel(kernel, NULL, globalWorkSize, localWorkSize);
	queue.enqueueReadBuffer(clOutput, CL_TRUE, 0, imageWidth * imageHeight * sizeof(cl_float3), cpuOutput);
	queue.enqueueReadBuffer(clStats, CL_TRUE, 0, sizeof(RenderStats), &stats);

	clock_t timeEnd = clock();
	stats.renderTime = (cl_float)(timeEnd - timeStart) / CLOCKS_PER_SEC;

	// Write render stats to console
	std::cout << std::endl;
	std::cout << "Render time:                " << stats.renderTime << " seconds" << std::endl;
	std::cout << "Primary rays:               " << stats.n_PrimaryRays << std::endl;
	std::cout << "Ray-triangle tests:         " << stats.n_RayTriangleTests << std::endl;
	std::cout << "Ray-triangle intersections: " << stats.n_RayTriangleIsects << std::endl;

	// Write image to .ppm file
	std::string outputPath = "output.ppm";
	std::cout << std::endl << "Writing to file \"" << outputPath << "\"..." << std::endl;
	std::ofstream outputFile;
	outputFile.open(outputPath);

	outputFile << "P3" << std::endl;
	outputFile << imageWidth << " " << imageHeight << std::endl;
	outputFile << "255" << std::endl;

	for (int i = 0; i < imageWidth * imageHeight; i++)
	{
		outputFile
			<< (int32_t)(cpuOutput[i].x * 255) << " "
			<< (int32_t)(cpuOutput[i].y * 255) << " "
			<< (int32_t)(cpuOutput[i].z * 255) << std::endl;
	}
	outputFile.close();
	std::cout << "Finished writing to file." << std::endl;

	delete[] cpuOutput;
}