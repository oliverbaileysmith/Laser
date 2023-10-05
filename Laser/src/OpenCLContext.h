#pragma once

#include <CL/cl.hpp>

#include <unordered_map>

class OpenCLContext
{
public:
	bool Init();
	bool LoadKernel(const std::string &filepath, const std::string &kernelName);

	bool AddBuffer(const std::string &bufferKey, cl_mem_flags clMemFlag,
		size_t size);

	bool SetKernelArg(cl_uint index, const std::string &bufferKey);
	bool SetKernelArg(cl_uint index, cl_int value);
	bool SetKernelArg(cl_uint index, cl_uint value);
	bool SetKernelArg(cl_uint index, cl_float value);
	bool SetKernelArg(cl_uint index, const cl_float3 &value);

	bool QueueWrite(const std::string &bufferKey, cl_bool blocking,
		size_t offset, size_t size, const void *data);
	bool QueueRead(const std::string &bufferKey, cl_bool blocking,
		size_t offset, size_t size, void *data);
	bool QueueKernel(const cl::NDRange &offset, const cl::NDRange &global,
		const cl::NDRange &local = cl::NullRange);

private:
	void PrintContextInfo();
	bool GetBuffer(const std::string &bufferKey, cl::Buffer &buffer);

	cl::Platform m_Platform;
	cl::Device m_Device;
	cl::Context m_Context;
	cl::Program m_Program;
	cl::Kernel m_Kernel;
	cl::CommandQueue m_CommandQueue;
	std::unordered_map<std::string, cl::Buffer> m_Buffers;
};
