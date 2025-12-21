#include "graphics/renderer.h"


#include <entt/signal/fwd.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <vector>

#include <SDL3/SDL_video.h>
#include <glad/glad.h>

#include "core/game_context.h"
#include "platform/window.h"
#include "events/resize_event.h"
#include "utils/read_file.h"
#include "components/text.h"
#include "components/text_mesh.h"


Renderer::Renderer(entt::registry* registry, Window* window)
  : _pRegistry(registry),
    _pWindow(window)
{
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();
  dispatcher.sink<ResizeEvent>().connect<&Renderer::onResize>(this);
}


Renderer::~Renderer()
{
  _pRegistry->view<Text, TextMesh>().each([this](Text& text, TextMesh& mesh){
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ebo);
  });

  if (_glCtx) SDL_GL_DestroyContext(_glCtx);
}


bool Renderer::Init()
{
  _glCtx = SDL_GL_CreateContext(_pWindow->GetNativeWindow());
  if (!_glCtx)
  {
    std::cerr << "[Renderer] Failed to create SDL_GLContext: " << SDL_GetError() << "\n";
    return false;
  }

  SDL_GL_MakeCurrent(_pWindow->GetNativeWindow(), _glCtx);

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
  {
    std::cerr << "[Renderer] Faile to load GL\n";
    return false;
  }

  auto& game_context = _pRegistry->ctx().get<GameContext>();
  glViewport(0, 0, game_context.screenInfo.width, game_context.screenInfo.height);

  return true;
}


void Renderer::UpdateFont()
{
  unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
  std::string vertex_source = ReadFile("assets/shaders/msdf_font.vert");
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
    std::cout << "[Renderer] Failed to compile vertex shader: " << info_log.data() << "\n";
  }

  unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
  std::string fragment_source = ReadFile("assets/shaders/msdf_font.frag");
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
    std::cout << "[Renderer] Failed to compile fragment shader: " << info_log.data() << "\n";
  }

  _program = glCreateProgram();
  glAttachShader(_program, vertex);
  glAttachShader(_program, fragment);
  glLinkProgram(_program);

  glGetProgramiv(_program, GL_LINK_STATUS, &success);
  if (!success)
  {
    int log_length;
    glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &log_length);
    std::vector<char> info_log(log_length);
    glGetProgramInfoLog(_program, log_length, &log_length, info_log.data());
    std::cout << "[Renderer] Failed to link program: " << info_log.data() << "\n";
  }

  auto& game_context = _pRegistry->ctx().get<GameContext>();
  _ortho = glm::ortho(0.0f, (float)game_context.screenInfo.width, 0.0f, (float)game_context.screenInfo.height, -1.0f, 1.0f);
}


void Renderer::BeginFrame()
{
  auto& screenInfo = _pRegistry->ctx().get<GameContext>().screenInfo;
  if (screenInfo.isMinimized) return;

  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}


void Renderer::EndFrame()
{
  if (_pWindow)
    _pWindow->SwapBuffers();
}


void Renderer::Render()
{
  // afficher l'UI à la fin
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  glUseProgram(_program);
  glUniformMatrix4fv(glGetUniformLocation(_program, "projection"), 1, GL_FALSE, &_ortho[0][0]);
  
  _pRegistry->view<Text, TextMesh>().each([this](Text& text, TextMesh& mesh){
    if (text.text.empty()) return;

    glBindTextureUnit(0, text.pFont->textureHandle);
    glUniform1f(glGetUniformLocation(_program, "pxRange"), text.pFont->pixelRange);
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
  });
}


void Renderer::onResize(const ResizeEvent& e)
{
  int width = e.width;
  int height = e.height;
  std::cout << "[Renderer] " << e.name << "[" << width << ", " << height << "]" << " called\n";
  glViewport(0, 0, width, height);
  _ortho = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);
}