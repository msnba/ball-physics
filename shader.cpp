#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath){
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER); // vertex shader
    glShaderSource(vShader, 1, &vShaderCode, nullptr);
    glCompileShader(vShader);
    checkCompileErrors(vShader, "VERTEX"); //cleaned up error checking

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER); // fragment shader
    glShaderSource(fShader, 1, &fShaderCode, nullptr);
    glCompileShader(fShader);
    checkCompileErrors(fShader, "FRAGMENT");

    id = glCreateProgram();
    glAttachShader(id, vShader);
    glAttachShader(id, fShader);
    glLinkProgram(id);
    checkCompileErrors(id, "PROGRAM");

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void Shader::use(){
    glUseProgram(id);
}

void Shader::setMat4(const std::string& name, const float* value){
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value);
}

void Shader::setVec4(const std::string& name, const float* value){
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, value);
}

std::string Shader::readFile(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type){ //streamlined error checking
    GLint success;
    GLchar infoLog[1024];
    if (type == "PROGRAM")
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << std::endl;
        }
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << std::endl;
        }
    }
}