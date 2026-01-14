#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

class Shader
{
public:
    Shader(const char *vertexPath, const char *fragmentPath);
    ~Shader();

    void use() const;
    unsigned int GetID() const { return ID; }

    // Utility uniform functions
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;

private:
    unsigned int ID = 0;
    mutable std::unordered_map<std::string, int> uniformLocationCache;

    void checkCompileErrors(unsigned int shader, std::string type);
    int getUniformLocation(const std::string &name) const;
};
#endif
