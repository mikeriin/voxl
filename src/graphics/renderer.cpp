#include "graphics/renderer.h"


#include <cstddef>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <cstring>
#include <vector>

#include <SDL3/SDL_video.h>
#include <glad/glad.h>

#include "components/font.h"
#include "components/font_mesh.h"
#include "core/game_context.h"
#include "platform/window.h"
#include "events/resize_event.h"
#include "utils/font_vertex.h"
#include "utils/read_file.h"


Renderer::Renderer(entt::registry* registry, Window* window)
  : _pRegistry(registry),
    _pWindow(window)
{
  auto& gameCtx = _pRegistry->ctx().get<GameContext>();
  gameCtx.dispatcher.sink<ResizeEvent>().connect<&Renderer::onResize>(this);
}


Renderer::~Renderer()
{
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

  auto& gameCtx = _pRegistry->ctx().get<GameContext>();
  glViewport(0, 0, gameCtx.screenInfo.width, gameCtx.screenInfo.height);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  return true;
}


void Renderer::UpdateFont()
{
  _pRegistry->view<Font, FontMesh>().each([](Font& font, FontMesh& mesh){
    glCreateVertexArrays(1, &mesh.vao);
    glCreateBuffers(1, &mesh.vbo);
    glCreateBuffers(1, &mesh.ebo);

    // stockage immuable pour la perf (mapping pour plus de rapidité)
    // on alloue assez pour écrire 512 caractères
    glNamedBufferStorage(mesh.vbo, MAX_TEXT_CHARACTERS * sizeof(FontVertex) * 4, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
    glVertexArrayVertexBuffer(mesh.vao, FONT_BINDING, mesh.vbo, 0, sizeof(FontVertex));
    glNamedBufferStorage(mesh.ebo, MAX_TEXT_CHARACTERS * sizeof(unsigned int) * 6, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
    glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

    // attributs position
    glEnableVertexArrayAttrib(mesh.vao, FONT_POSITION_LOC);
    glVertexArrayAttribFormat(mesh.vao, FONT_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(FontVertex, position));
    glVertexArrayAttribBinding(mesh.vao, FONT_POSITION_LOC, FONT_BINDING);

    // attributs uv
    glEnableVertexArrayAttrib(mesh.vao, FONT_UV_LOC);
    glVertexArrayAttribFormat(mesh.vao, FONT_UV_LOC, 2, GL_FLOAT, GL_FALSE, offsetof(FontVertex, uv));
    glVertexArrayAttribBinding(mesh.vao, FONT_UV_LOC, FONT_BINDING);

    size_t vertexBytes = mesh.vertices.size() * sizeof(FontVertex);
    size_t indexBytes = mesh.indices.size() * sizeof(unsigned int);

    void* vertexPtr = glMapNamedBufferRange(mesh.vbo, 0, vertexBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (vertexPtr)
    {
      std::memcpy(vertexPtr, mesh.vertices.data(), vertexBytes);
      glUnmapNamedBuffer(mesh.vbo);
      std::cout << "[Renderer] Vertex data successfully copied\n";
    }

    void* indexPtr = glMapNamedBufferRange(mesh.ebo, 0, indexBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (indexPtr)
    {
      std::memcpy(indexPtr, mesh.indices.data(), indexBytes);
      glUnmapNamedBuffer(mesh.ebo);
      std::cout << "[Renderer] Index data successfully copied\n";
    }
  });

  unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
  std::string vertSource = ReadFile("assets/shaders/msdf_font.vert");
  const char* vertSourceC = vertSource.c_str();
  glShaderSource(vert, 1, &vertSourceC, nullptr);
  glCompileShader(vert);

  int success;
  glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    int logLength;
    glGetShaderiv(vert, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> infoLog(logLength);
    glGetShaderInfoLog(vert, logLength, &logLength, infoLog.data());
    std::cout << "[Renderer] Failed to compile vertex shader: " << infoLog.data() << "\n";
  }

  unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
  std::string fragSource = ReadFile("assets/shaders/msdf_font.frag");
  const char* fragSourceC = fragSource.c_str();
  glShaderSource(frag, 1, &fragSourceC, nullptr);
  glCompileShader(frag);

  glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    int logLength;
    glGetShaderiv(frag, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> infoLog(logLength);
    glGetShaderInfoLog(frag, logLength, &logLength, infoLog.data());
    std::cout << "[Renderer] Failed to compile fragment shader: " << infoLog.data() << "\n";
  }

  _program = glCreateProgram();
  glAttachShader(_program, vert);
  glAttachShader(_program, frag);
  glLinkProgram(_program);

  glGetProgramiv(_program, GL_LINK_STATUS, &success);
  if (!success)
  {
    int logLength;
    glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> infoLog(logLength);
    glGetProgramInfoLog(_program, logLength, &logLength, infoLog.data());
    std::cout << "[Renderer] Failed to link program: " << infoLog.data() << "\n";
  }

  auto& gameCtx = _pRegistry->ctx().get<GameContext>();
  _ortho = glm::ortho(0.0f, (float)gameCtx.screenInfo.width, (float)gameCtx.screenInfo.height, 0.0f, -1.0f, 1.0f);
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
  glDisable(GL_DEPTH_TEST);
  glUseProgram(_program);
  glUniform1f(glGetUniformLocation(_program, "pxRange"), 2.0f);
  glUniformMatrix4fv(glGetUniformLocation(_program, "projection"), 1, GL_FALSE, &_ortho[0][0]);
  
  _pRegistry->view<Font, FontMesh>().each([this](Font& font, FontMesh& mesh){
    glUniform1i(glGetUniformLocation(_program, "msdfTex"), font.texID);
    glBindTextureUnit(0, font.texID);
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
  _ortho = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
}