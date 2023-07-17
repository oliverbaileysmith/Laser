#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "OpenCLContext.h"
#include "Image.h"
#include "Camera.h"
#include "RenderStats.h"
#include "TriangleMesh.h"
#include "Material.h"
#include "BVH.h"

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
	std::vector<Material> m_Materials;
	std::vector<glm::mat4> m_Transforms;
	BVH m_BVH;

	// Image and camera
	Image m_Image;
	Camera m_Camera;
	
	// Profiler
	RenderStats m_RenderStats;
	clock_t m_AppStart;
	clock_t m_AppEnd;
	clock_t m_RenderStart;
	clock_t m_RenderEnd;
};