#include <iostream>
#include <fstream>
#include <vector>

#include <CL/cl.hpp>

struct Triangle
{
	cl_uint v0;
	cl_uint v1;
	cl_uint v2;
};

struct RenderStats
{
	cl_uint n_PrimaryRays = 0;
	cl_uint n_RayTriangleTests = 0;
	cl_uint n_RayTriangleIsects = 0;
	cl_float RenderTime = 0.0f; // Time in seconds
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
		std::cout << std::endl << "OpenCL program compilation error: " << buildError << std::endl;
		std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << "Build log:" << std::endl << buildLog << std::endl;
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

	RenderStats stats;

	// Temporary dummy mesh vertices and indices
	cl_float3 vertices[8];
	vertices[0] = { -0.5f, -0.5f,  0.2f }; // front bottom left
	vertices[1] = { -0.5f, -0.5f, -0.8f }; // back bottom left
	vertices[2] = { -0.5f,  0.5f,  0.2f }; // front top left
	vertices[3] = { -0.5f,  0.5f, -0.8f }; // back top left
	vertices[4] = {  0.5f, -0.5f,  0.2f }; // front bottom right
	vertices[5] = {  0.5f, -0.5f, -0.8f }; // back bottom right
	vertices[6] = {  0.5f,  0.5f,  0.2f }; // front top right
	vertices[7] = {  0.5f,  0.5f, -0.8f }; // back top right

	unsigned int n_Triangles = 10;
	Triangle triangles[10];
	triangles[0] = { 0, 1, 3 }; // left
	triangles[1] = { 0, 3, 2 };
	triangles[2] = { 1, 5, 7 }; // back
	triangles[3] = { 1, 7, 3 };
	triangles[4] = { 5, 4, 6 }; // right
	triangles[5] = { 5, 6, 7 };
	triangles[6] = { 4, 0, 2 }; // front
	triangles[7] = { 4, 2, 6 };
	triangles[8] = { 0, 4, 5 }; // bottom
	triangles[9] = { 0, 5, 1 };
	//triangles[10] = { 3, 7, 6 }; // top
	//triangles[11] = { 3, 6, 2 };
	

	// OpenCL device data
	cl::Buffer clOutput(context, CL_MEM_WRITE_ONLY, imageWidth * imageHeight * sizeof(cl_float3));
	cl::Buffer clStats(context, CL_MEM_READ_WRITE, sizeof(RenderStats));
	cl::Buffer clVertices(context, CL_MEM_READ_ONLY, sizeof(vertices));
	cl::Buffer clTriangles(context, CL_MEM_READ_ONLY, sizeof(triangles));

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
	kernel.setArg(12, clStats);

	// OpenCL command queue
	cl::CommandQueue queue(context, device);

	// OpenCL work items
	std::size_t globalWorkSize = imageWidth * imageHeight;
	std::size_t localWorkSize = 64;

	clock_t timeStart = clock();

	// Queue kernel execution and read result from device buffer
	queue.enqueueWriteBuffer(clVertices, CL_TRUE, 0, sizeof(vertices), &vertices);
	queue.enqueueWriteBuffer(clTriangles, CL_TRUE, 0, sizeof(triangles), &triangles);
	queue.enqueueNDRangeKernel(kernel, NULL, globalWorkSize, localWorkSize);
	queue.enqueueReadBuffer(clOutput, CL_TRUE, 0, imageWidth * imageHeight * sizeof(cl_float3), cpuOutput);
	queue.enqueueReadBuffer(clStats, CL_TRUE, 0, sizeof(RenderStats), &stats);

	clock_t timeEnd = clock();
	stats.RenderTime = (cl_float)(timeEnd - timeStart) / CLOCKS_PER_SEC;

	// Write render stats to console
	std::cout << std::endl;
	std::cout << "Render time:                " << stats.RenderTime << " seconds" << std::endl;
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