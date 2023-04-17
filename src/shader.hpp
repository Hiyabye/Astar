#ifndef __SHADER_HPP__
#define __SHADER_HPP__

class Shader {
public:
  unsigned int ID;

  unsigned int compileShader(const char* shaderSource, GLenum shaderType);
  unsigned int compileProgram(unsigned int vertexShader, unsigned int fragmentShader);
  Shader(const char* vertexPath, const char* fragmentPath);
  ~Shader();
};

#endif