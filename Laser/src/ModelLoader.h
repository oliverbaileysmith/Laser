#pragma once

#include <assimp/scene.h>

#include "TriangleMesh.h"

class ModelLoader
{
public:
	bool LoadModel(const std::string &filepath,
		std::vector<TriangleMesh> &meshes, unsigned int materialIndex,
		unsigned int transformIndex);

private:
	void ProcessAssimpNode(aiNode *assimpNode, const aiScene *assimpScene,
		std::vector<TriangleMesh> &meshes, unsigned int materialIndex,
		unsigned int transformIndex, size_t vertexOffset);

	TriangleMesh ProcessAssimpMesh(aiMesh *assimpMesh,
		const aiScene *assimpScene, unsigned int materialIndex,
		unsigned int transformIndex, size_t vertexOffset);
};
