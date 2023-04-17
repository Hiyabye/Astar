#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "shader.hpp"

// Compile shader from file
unsigned int Shader::compileShader(const char* shaderPath, GLenum shaderType) {
  int success;
  char infoLog[512];
  char shaderTypeName[10];

  // Get shader type name
  switch (shaderType) {
    case GL_VERTEX_SHADER: strcpy(shaderTypeName, "Vertex"); break;
    case GL_FRAGMENT_SHADER: strcpy(shaderTypeName, "Fragment"); break;
    default: strcpy(shaderTypeName, "Unknown");
  }

  // Read shader source from file
  std::string shaderCode;
  std::ifstream shaderFile;
  shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    shaderFile.open(shaderPath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    shaderCode = shaderStream.str();
  } catch (std::ifstream::failure e) {
    std::cout << "[ERROR] " << shaderTypeName << " shader file not successfully read" << std::endl;
  }

  // Compile shader
  const char* shaderSource = shaderCode.c_str();
  unsigned int shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderSource, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "[ERROR] " << shaderTypeName << " shader compilation failed" << std::endl;
    std::cout << infoLog << std::endl;
  }
  std::cout << "[DEBUG] " << shaderTypeName << " shader compiled successfully" << std::endl;
  return shader;
}

// Link shaders into a program
unsigned int Shader::compileProgram(unsigned int vertexShader, unsigned int fragmentShader) {
  int success;
  char infoLog[512];

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "[ERROR] Shader program linking failed" << std::endl;
    std::cout << infoLog << std::endl;
  }
  std::cout << "[DEBUG] Shader program linked successfully" << std::endl;
  return shaderProgram;
}

// Constructor
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
  unsigned int vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
  unsigned int fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);
  ID = compileProgram(vertexShader, fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

// Destructor
Shader::~Shader() {
  glDeleteProgram(ID);
}