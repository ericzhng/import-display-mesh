#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <limits>

// Forward declarations for Assimp types
struct aiNode;
struct aiScene;
struct aiMesh;

class Model
{
public:
    Model(const char *path);
    void Draw() const;
    glm::vec3 GetCenter();

private:
    std::vector<Mesh> meshes;
    glm::vec3 BoundingBoxMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 BoundingBoxMax = glm::vec3(std::numeric_limits<float>::lowest());

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};
#endif