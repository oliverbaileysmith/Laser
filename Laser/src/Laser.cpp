#include <iostream>
#include <fstream>
#include <vector>

#include <CL/cl.hpp>

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

	// OpenCL context
	cl::Context context(device);

	// OpenCL kernel source
	const char* src =
		"struct Ray"
		"{"
		"float3 m_Origin;"
		"float3 m_Direction;"
		"};"
		"__kernel void Laser(__global float3* output, int imageWidth, int imageHeight, float aspectRatio, float viewportWidth, float viewportHeight, float focalLength, float3 cameraOrigin, float3 lowerLeftCorner)"
		"{"
		"const int workItemID = get_global_id(0);"
		"int x = workItemID % imageWidth;"
		"int y = workItemID / imageWidth;"
		"float fx = (float)x / (float)(imageWidth - 1);"
		"float fy = (float)y / (float)(imageHeight - 1);"
		"struct Ray ray;"
		"ray.m_Origin = cameraOrigin;"
		"ray.m_Direction = lowerLeftCorner;"
		"ray.m_Direction.x += fx * viewportWidth;"
		"ray.m_Direction.y += fy * viewportHeight;"
		"ray.m_Direction -= cameraOrigin;"
		"ray.m_Direction = normalize(ray.m_Direction);"
		"output[workItemID] = fabs(ray.m_Direction);"
		"}";

	// OpenCL program
	cl::Program program(context, src);

	cl_int buildError = program.build({ device }, "");
	if (buildError)
	{
		std::cout << "OpenCL program compilation error: " << buildError << std::endl;
		return -1;
	}

	// OpenCL kernel
	cl::Kernel kernel(program, "Laser");

	// Host data
	const int imageWidth = 1280;
	const int imageHeight = 720;
	const float aspectRatio = (float)imageWidth / (float)imageHeight;
	float viewportHeight = 2.0f;
	float viewportWidth = viewportHeight * aspectRatio;
	float focalLength = 1.0f;
	cl_float3 cameraOrigin = { 0.0f,0.0f,0.0f };
	cl_float3 lowerLeftCorner = cameraOrigin;
	lowerLeftCorner.x -= viewportWidth / 2.0f;
	lowerLeftCorner.y -= viewportHeight / 2.0f;
	lowerLeftCorner.z -= focalLength;

	cl_float3* cpuOutput = new cl_float3[imageWidth * imageHeight];

	// OpenCL device data
	cl::Buffer clOutput(context, CL_MEM_WRITE_ONLY, imageWidth * imageHeight * sizeof(cl_float3));

	// OpenCL kernel arguments
	kernel.setArg(0, clOutput);
	kernel.setArg(1, imageWidth);
	kernel.setArg(2, imageHeight);
	kernel.setArg(3, aspectRatio);
	kernel.setArg(4, viewportWidth);
	kernel.setArg(5, viewportHeight);
	kernel.setArg(6, focalLength);
	kernel.setArg(7, cameraOrigin);
	kernel.setArg(8, lowerLeftCorner);

	// OpenCL command queue
	cl::CommandQueue queue(context, device);

	// OpenCL work items
	std::size_t globalWorkSize = imageWidth * imageHeight;
	std::size_t localWorkSize = 64;

	// Queue kernel execution and read result from device buffer
	queue.enqueueNDRangeKernel(kernel, NULL, globalWorkSize, localWorkSize);
	queue.enqueueReadBuffer(clOutput, CL_TRUE, 0, imageWidth * imageHeight * sizeof(cl_float3), cpuOutput);

	// Write image to .ppm file
	std::cout << "Writing to file...";
	std::ofstream outputFile;
	outputFile.open("output.ppm");

	outputFile << "P3" << std::endl;
	outputFile << imageWidth << " " << imageHeight << std::endl;
	outputFile << "255" << std::endl;

	for (int i = 0; i < imageWidth * imageHeight; i++)
	{
		outputFile
			<< (int)(cpuOutput[i].x * 255) << " "
			<< (int)(cpuOutput[i].y * 255) << " "
			<< (int)(cpuOutput[i].z * 255) << std::endl;
	}
	outputFile.close();

	delete[] cpuOutput;
}