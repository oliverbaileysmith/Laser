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
	bool LoadModel(const std::string& filepath, std::vector<TriangleMesh>& meshes,
		unsigned int materialIndex, unsigned int transformIndex);
	void CombineMeshes(std::vector<TriangleMesh>& meshes, std::vector<Vertex>& vertices,
		std::vector<Triangle>& triangles);

	// OpenCL context
	OpenCLContext m_OCL;
	size_t m_GlobalWorkSize;
	size_t m_LocalWorkSize;

	// Scene
	std::vector<Material> m_Materials;
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