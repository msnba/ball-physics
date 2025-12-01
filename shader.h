#pragma once
#include <string>
#include <glad/glad.h>

class Shader{
    public:
    GLuint id;

    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    
    void use();
    void setMat4(const std::string& name, const float* value);
    void setVec4(const std::string& name, const float* value);

    private:
    std::string readFile(const std::string& path);
    void checkCompileErrors(GLuint shader, const std::string& type);
};