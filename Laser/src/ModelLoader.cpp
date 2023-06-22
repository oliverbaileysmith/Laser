#include "ModelLoader.h"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

std::vector<TriangleMesh> ModelLoader::LoadModel(const std::string& filepath)
{
    std::vector<TriangleMesh> meshes;
    Assimp::Importer importer;

    // Read file and triangulate meshes
    const aiScene* assimpScene = importer.ReadFile(filepath, aiProcess_Triangulate);

    // Ensure scene is created correctly
    if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
    {
        std::cout << "Error loading model at " << filepath << "." << std::endl;
        std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
        return std::vector<TriangleMesh>();
    }

    // Process first node
    ProcessAssimpNode(assimpScene->mRootNode, assimpScene, meshes);
    return meshes;
}

void ModelLoader::ProcessAssimpNode(aiNode* assimpNode, const aiScene* assimpScene, std::vector<TriangleMesh>& meshes)
{
    // Process meshes in node
    for (uint32_t i = 0; i < assimpNode->mNumMeshes; i++)
    {
        aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
        meshes.push_back(ProcessAssimpMesh(assimpMesh, assimpScene));
    }
    // Recursively process child nodes
    for (uint32_t i = 0; i < assimpNode->mNumChildren; i++)
    {
        ProcessAssimpNode(assimpNode->mChildren[i], assimpScene, meshes);
    }
}

TriangleMesh ModelLoader::ProcessAssimpMesh(aiMesh* assimpMesh, const aiScene* assimpScene)
{
    std::vector<cl_float3> vertices;
    std::vector<Triangle> triangles;

    // Process vertex positions
    for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
    {
        cl_float3 vertex;

        // TODO: remove manual transform of vertices
        vertex.x = assimpMesh->mVertices[i].x / 1.8;
        vertex.y = assimpMesh->mVertices[i].y / 1.8 - 1.5f;
        vertex.z = assimpMesh->mVertices[i].z / 1.8 - 1.5f;

        vertices.push_back(vertex);
    }

    // Process indices
    for (uint32_t i = 0; i < assimpMesh->mNumFaces; i++)
    {
        Triangle triangle;
        aiFace assimpFace = assimpMesh->mFaces[i];
        
        triangle.v0 = assimpFace.mIndices[0];
        triangle.v1 = assimpFace.mIndices[1];
        triangle.v2 = assimpFace.mIndices[2];

        // TODO: correctly assign material
        triangle.MaterialIndex = 0;

        triangles.push_back(triangle);
    }

    return TriangleMesh(vertices, triangles);
}