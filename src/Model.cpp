#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

Model::Model(const char *path)
{
    loadModel(path);
}

void Model::Draw() const
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw();
}

glm::vec3 Model::GetCenter()
{
    return (BoundingBoxMin + BoundingBoxMax) / 2.0f;
}

glm::vec3 Model::GetSize()
{
    return BoundingBoxMax - BoundingBoxMin;
}

void Model::loadModel(std::string path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    if (!mesh->HasNormals()) {
        std::cerr << "WARNING: Mesh '" << mesh->mName.C_Str() << "' does not have normals after Assimp processing, generating default (0,0,1)." << std::endl;
    }
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        BoundingBoxMin.x = std::min(BoundingBoxMin.x, vector.x);
        BoundingBoxMin.y = std::min(BoundingBoxMin.y, vector.y);
        BoundingBoxMin.z = std::min(BoundingBoxMin.z, vector.z);

        BoundingBoxMax.x = std::max(BoundingBoxMax.x, vector.x);
        BoundingBoxMax.y = std::max(BoundingBoxMax.y, vector.y);
        BoundingBoxMax.z = std::max(BoundingBoxMax.z, vector.z);

        if (mesh->HasNormals())
        {
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }
        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    return Mesh(vertices, indices);
}
