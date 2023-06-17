#pragma once

#include <assimp/scene.h>

#include "TriangleMesh.h"

class ModelLoader
{
public:
	std::vector<TriangleMesh> LoadModel(std::string filepath);

private:
	void ProcessAssimpNode(aiNode* assimpNode, const aiScene* assimpScene, std::vector<TriangleMesh>& meshes);
	TriangleMesh ProcessAssimpMesh(aiMesh* assimpMesh, const aiScene* assimpScene);
};