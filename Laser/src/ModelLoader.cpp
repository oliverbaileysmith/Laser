#include "ModelLoader.h"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Vertex.h"

bool ModelLoader::LoadModel(const std::string& filepath, std::vector<TriangleMesh>& meshes,
    unsigned int materialIndex, unsigned int transformIndex)
{
    // Used to assign scene-wide indices
    static size_t previousMeshes = 0;
    static size_t vertexOffset = 0;

    Assimp::Importer importer;

    // Read file and triangulate meshes
    const aiScene* assimpScene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    // Ensure scene is created correctly
    if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
    {
        std::cout << "Error loading model at " << filepath << "." << std::endl;
        std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    // Process first node
    ProcessAssimpNode(assimpScene->mRootNode, assimpScene, meshes, materialIndex, transformIndex, vertexOffset);

    // For each new mesh from this call of LoadModel
    for (int i = previousMeshes; i < meshes.size(); i++)
    {
        // Increase vertex offset for indexing
        vertexOffset += meshes[i].m_Vertices.size();
    }
    previousMeshes = meshes.size();
    return true;
}

void ModelLoader::ProcessAssimpNode(aiNode* assimpNode, const aiScene* assimpScene,
    std::vector<TriangleMesh>& meshes, unsigned int materialIndex,
    unsigned int transformIndex, size_t vertexOffset)
{
    // Process meshes in node
    for (uint32_t i = 0; i < assimpNode->mNumMeshes; i++)
    {
        aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
        meshes.push_back(ProcessAssimpMesh(assimpMesh, assimpScene, materialIndex, transformIndex, vertexOffset));
    }
    // Recursively process child nodes
    for (uint32_t i = 0; i < assimpNode->mNumChildren; i++)
    {
        ProcessAssimpNode(assimpNode->mChildren[i], assimpScene, meshes, materialIndex, transformIndex, vertexOffset);
    }
}

TriangleMesh ModelLoader::ProcessAssimpMesh(aiMesh* assimpMesh, const aiScene* assimpScene,
    unsigned int materialIndex, unsigned int transformIndex, size_t vertexOffset)
{
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;

    // Process vertex positions
    for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Position.x = assimpMesh->mVertices[i].x;
        vertex.Position.y = assimpMesh->mVertices[i].y;
        vertex.Position.z = assimpMesh->mVertices[i].z;

        if (assimpMesh->mNormals)
        {
            vertex.Normal.x = assimpMesh->mNormals[i].x;
            vertex.Normal.y = assimpMesh->mNormals[i].y;
            vertex.Normal.z = assimpMesh->mNormals[i].z;
        }

        vertices.push_back(vertex);
    }

    // Process indices
    for (uint32_t i = 0; i < assimpMesh->mNumFaces; i++)
    {
        Triangle triangle;
        aiFace assimpFace = assimpMesh->mFaces[i];
        
        triangle.v0 = assimpFace.mIndices[0] + vertexOffset;
        triangle.v1 = assimpFace.mIndices[1] + vertexOffset;
        triangle.v2 = assimpFace.mIndices[2] + vertexOffset;

        triangle.Material = materialIndex;
        triangle.Transform = transformIndex;

        triangles.push_back(triangle);
    }

    return TriangleMesh(vertices, triangles);
}