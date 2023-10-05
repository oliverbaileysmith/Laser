#include "OpenCLContext.h"

#include <iostream>
#include <fstream>
#include <vector>

bool OpenCLContext::Init()
{
	// Platform
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	if (platforms.empty())
	{
		std::cout << "No OpenCL platform available." << std::endl;
		return false;
	}

	m_Platform = platforms[0];

	// Device
	std::vector<cl::Device> devices;
	m_Platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

	if (devices.empty())
	{
		std::cout << "No OpenCL GPU device available." << std::endl;
		return false;
	}

	m_Device = devices[0];

	// Context
	m_Context = cl::Context(m_Device);

	// Command queue
	m_CommandQueue = cl::CommandQueue(m_Context, m_Device);

	PrintContextInfo();
	return true;
}

bool OpenCLContext::LoadKernel(const std::string &filepath,
	const std::string &kernelName)
{
	// Read kernel source
	std::string kernelSrc;
	std::string line;
	std::ifstream kernelFile(filepath);

	if (!kernelFile.good())
	{
		std::cout << "Failed to open file at " << filepath << std::endl;
		return false;
	}

	while (std::getline(kernelFile, line))
	{
		kernelSrc += line + '\n';
	}

	kernelFile.close();

	// Build program
	m_Program = cl::Program(m_Context, kernelSrc.c_str());

	cl_int buildError = m_Program.build({m_Device}, "-I cl");
	if (buildError)
	{
		std::cout << std::endl
				  << "OpenCL program compilation error: " << buildError
				  << std::endl;
		std::string buildLog =
			m_Program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_Device);
		std::cout << "Build log:" << std::endl << buildLog << std::endl;
		return false;
	}

	// Kernel object
	m_Kernel = cl::Kernel(m_Program, kernelName.c_str());
	return true;
}

bool OpenCLContext::AddBuffer(const std::string &bufferKey,
	cl_mem_flags clMemFlag, size_t size)
{
	if (m_Buffers.find(bufferKey) != m_Buffers.end())
	{
		std::cout << "Buffer with name " << bufferKey << " already exists."
				  << std::endl;
		return false;
	}

	if (size < 0)
		return false;

	m_Buffers[bufferKey] = cl::Buffer(m_Context, clMemFlag, size);
	return true;
}

bool OpenCLContext::SetKernelArg(cl_uint index, const std::string &bufferKey)
{
	if (m_Buffers.find(bufferKey) == m_Buffers.end())
	{
		std::cout << "No buffer with name \"" << bufferKey << "\" found."
				  << std::endl;
		return false;
	}

	cl_int kernelError = m_Kernel.setArg(index, m_Buffers[bufferKey]);
	if (kernelError)
	{
		std::cout << "OpenCL kernel error: " << kernelError << std::endl;
		return false;
	}
	return true;
}

bool OpenCLContext::SetKernelArg(cl_uint index, cl_int value)
{
	cl_int kernelError = m_Kernel.setArg(index, value);
	if (kernelError)
	{
		std::cout << "OpenCL kernel error: " << kernelError << std::endl;
		return false;
	}
	return true;
}

bool OpenCLContext::SetKernelArg(cl_uint index, cl_uint value)
{
	cl_int kernelError = m_Kernel.setArg(index, value);
	if (kernelError)
	{
		std::cout << "OpenCL kernel error: " << kernelError << std::endl;
		return false;
	}
	return true;
}

bool OpenCLContext::SetKernelArg(cl_uint index, cl_float value)
{
	cl_int kernelError = m_Kernel.setArg(index, value);
	if (kernelError)
	{
		std::cout << "OpenCL kernel error: " << kernelError << std::endl;
		return false;
	}
	return true;
}

bool OpenCLContext::SetKernelArg(cl_uint index, const cl_float3 &value)
{
	cl_int kernelError = m_Kernel.setArg(index, value);
	if (kernelError)
	{
		std::cout << "OpenCL kernel error: " << kernelError << std::endl;
		return false;
	}
	return true;
}

bool OpenCLContext::QueueWrite(const std::string &bufferKey, cl_bool blocking,
	size_t offset, size_t size, const void *data)
{
	cl::Buffer buffer;
	if (!GetBuffer(bufferKey, buffer))
	{
		std::cout << "Could not write to non-existent buffer \"" << bufferKey
				  << "\"." << std::endl;
		return false;
	}

	cl_int queueError =
		m_CommandQueue.enqueueWriteBuffer(buffer, blocking, offset, size, data);
	if (queueError)
	{
		std::cout << "OpenCL command queue error: " << queueError << std::endl;
		return false;
	}
	return true;
}

bool OpenCLContext::QueueRead(const std::string &bufferKey, cl_bool blocking,
	size_t offset, size_t size, void *data)
{
	cl::Buffer buffer;
	if (!GetBuffer(bufferKey, buffer))
	{
		std::cout << "Could not read from non-existent buffer \"" << bufferKey
				  << "\"." << std::endl;
		return false;
	}

	cl_int queueError =
		m_CommandQueue.enqueueReadBuffer(buffer, blocking, offset, size, data);
	if (queueError)
	{
		std::cout << "OpenCL command queue error: " << queueError << std::endl;
		return false;
	}
	return true;
}

bool OpenCLContext::QueueKernel(const cl::NDRange &offset,
	const cl::NDRange &global, const cl::NDRange &local)
{
	cl_int queueError =
		m_CommandQueue.enqueueNDRangeKernel(m_Kernel, offset, global, local);
	if (queueError)
	{
		std::cout << "OpenCL command queue error: " << queueError << std::endl;
		return false;
	}
	return true;
}

void OpenCLContext::PrintContextInfo()
{
	std::cout << "OpenCL platform: " << m_Platform.getInfo<CL_PLATFORM_NAME>()
			  << std::endl;
	std::cout << "OpenCL device: " << m_Device.getInfo<CL_DEVICE_NAME>()
			  << std::endl;
	std::cout << "OpenCL version: " << m_Device.getInfo<CL_DEVICE_VERSION>()
			  << std::endl
			  << std::endl;
}

bool OpenCLContext::GetBuffer(const std::string &bufferKey, cl::Buffer &buffer)
{
	if (m_Buffers.find(bufferKey) != m_Buffers.end())
	{
		buffer = m_Buffers[bufferKey];
		return true;
	}

	std::cout << "No buffer with name \"" << bufferKey << "\" found."
			  << std::endl;
	return false;
}
