#ifndef VOXL_SHADER_LOADER_H
#define VOXL_SHADER_LOADER_H


#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>

#include "resources/shader.h"
#include "utils/read_file.h"


struct ShaderLoader
{
  using result_type = std::shared_ptr<Shader>;

  result_type operator()(const std::string& name)
  {
    Shader shader;

    std::string base = "assets/shaders/";

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    std::string vertex_source = ReadFile(base + name + ".vert");
    const char* vertex_source_c = vertex_source.c_str();
    glShaderSource(vertex, 1, &vertex_source_c, nullptr);
    glCompileShader(vertex);

    int success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      int log_length;
      glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &log_length);
      std::vector<char> info_log(log_length);
      glGetShaderInfoLog(vertex, log_length, &log_length, info_log.data());
      std::cout << "[LoadShader] " << name << " - Failed to compile vertex shader: " << info_log.data() << "\n";
      
      return nullptr;
    }

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragment_source = ReadFile(base + name + ".frag");
    const char* fragment_source_c = fragment_source.c_str();
    glShaderSource(fragment, 1, &fragment_source_c, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      int log_length;
      glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &log_length);
      std::vector<char> info_log(log_length);
      glGetShaderInfoLog(fragment, log_length, &log_length, info_log.data());
      std::cout << "[LoadShader] " << name << " - Failed to compile fragment shader: " << info_log.data() << "\n";
      
      return nullptr;
    }

    shader.program = glCreateProgram();
    glAttachShader(shader.program, vertex);
    glAttachShader(shader.program, fragment);
    glLinkProgram(shader.program);

    glGetProgramiv(shader.program, GL_LINK_STATUS, &success);
    if (!success)
    {
      int log_length;
      glGetProgramiv(shader.program, GL_INFO_LOG_LENGTH, &log_length);
      std::vector<char> info_log(log_length);
      glGetProgramInfoLog(shader.program, log_length, &log_length, info_log.data());
      std::cout << "[LoadShader] " << name << " - Failed to link program: " << info_log.data() << "\n";

      return nullptr;
    }

    return std::make_shared<Shader>(shader);
  }
};


#endif // !VOXL_SHADER_LOADER_H