#pragma once

#include <assimp/scene.h>

#include "TriangleMesh.h"

class ModelLoader
{
public:
	bool LoadModel(const std::string& filepath, std::vector<TriangleMesh>& meshes);

private:
	void ProcessAssimpNode(aiNode* assimpNode, const aiScene* assimpScene, std::vector<TriangleMesh>& meshes);
	TriangleMesh ProcessAssimpMesh(aiMesh* assimpMesh, const aiScene* assimpScene);
};