#pragma once

#include <vector>

#include "OpenCLContext.h"
#include "Image.h"
#include "RenderStats.h"
#include "TriangleMesh.h"
#include "Material.h"

class Application
{
public:
	Application();
	
	bool Init();
	bool GenBuffers();
	bool SetKernelArgs();
	bool Render();
	bool WriteOutput();

private:
	bool LoadModel(const std::string& filepath);

	// OpenCL context
	OpenCLContext m_OCL;
	size_t m_GlobalWorkSize;
	size_t m_LocalWorkSize;

	// Scene
	std::vector<TriangleMesh> m_Meshes;
	cl_uint m_NTriangles;
	std::vector<Material> m_Materials;

	// Image
	Image m_Image;

	// Camera
	// TODO: Camera abstraction
	cl_float m_ViewportHeight;
	cl_float m_ViewportWidth;
	cl_float m_FocalLength;
	cl_float3 m_CameraOrigin;
	cl_float3 m_UpperLeftCorner;
	
	// Profiler
	RenderStats m_RenderStats;
};