#include "graphics/renderer.h"


#include <entt/signal/fwd.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <vector>

#include <SDL3/SDL_video.h>
#include <glad/glad.h>

#include "core/console_context.h"
#include "core/game_context.h"
#include "core/command_manager.h"
#include "core/command.h"
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
  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

  registerCommands();

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


void Renderer::registerCommands()
{
  auto& command_manager = _pRegistry->ctx().get<CommandManager>();
  auto& console_context = _pRegistry->ctx().get<ConsoleContext>();

  std::string helper = "?> $set_clear_color <red> <green> <blue> -- color values must be between 0 and 1 --> change GL clear color buffer";
  std::string name = "set_clear_color";
  command_manager.Register(Command{
    .name = name,
    .helper = helper,
    .func = [name, helper, &console_context](auto& args)
    {
      try {
        if (args.size() != 3) throw std::out_of_range("[Engine] $set_clear_color needs 3 args");
        
        size_t last_valid_index;
        float r = std::stof(args[0], &last_valid_index);
        if (last_valid_index != args[0].size() || r < 0.0f || r > 1.0f) throw std::invalid_argument("[Engine] args[0] must be between 0 and 1 included");
        float g = std::stof(args[1], &last_valid_index);
        if (last_valid_index != args[1].size() || g < 0.0f || g > 1.0f) throw std::invalid_argument("[Engine] args[1] must be between 0 and 1 included");
        float b = std::stof(args[2], &last_valid_index);
        if (last_valid_index != args[2].size() || b < 0.0f || b > 1.0f) throw std::invalid_argument("[Engine] args[2] must be between 0 and 1 included");

        glClearColor(r, g, b, 1.0f);
      } 
      // la dite erreur
      catch (const std::out_of_range& e) 
      {
        console_context.historyBuffer.insert_or_assign(name, helper);
        std::cerr << e.what() << "\n";
      }
      catch (const std::invalid_argument& e)
      {
        console_context.historyBuffer.insert_or_assign(name, helper);
        std::cerr << e.what() << "\n";
      }
    }
  });


  helper = "?> $set_clear_color_rgb <red> <green> <blue> -- color values must be between 0 and 255 --> change GL clear color buffer";
 name = "set_clear_color_rgb";
  command_manager.Register(Command{
    .name = name,
    .helper = helper,
    .func = [name, helper, &console_context](auto& args)
    {
      try {
        if (args.size() != 3) throw std::out_of_range("[Engine] $set_clear_color_rgb needs 3 args");
        
        size_t last_valid_index;
        int r = std::stoi(args[0], &last_valid_index);
        if (last_valid_index != args[0].size() || r < 0 || r > 255) throw std::invalid_argument("[Engine] args[0] must be between 0 and 255 included");
        int g = std::stoi(args[1], &last_valid_index);
        if (last_valid_index != args[1].size() || g < 0 || g > 255) throw std::invalid_argument("[Engine] args[1] must be between 0 and 255 included");
        int b = std::stoi(args[2], &last_valid_index);
        if (last_valid_index != args[2].size() || b < 0 || b > 255) throw std::invalid_argument("[Engine] args[2] must be between 0 and 255 included");

        glClearColor((float) r / 255.0f, (float) g / 255.0f, (float) b / 255.0f, 1.0f);
      } 
      // la dite erreur
      catch (const std::out_of_range& e) 
      {
        console_context.historyBuffer.insert_or_assign(name, helper);
        std::cerr << e.what() << "\n";
      }
      catch (const std::invalid_argument& e)
      {
        console_context.historyBuffer.insert_or_assign(name, helper);
        std::cerr << e.what() << "\n";
      }
    }
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