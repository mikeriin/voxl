#include "graphics/renderer.h"


#include <entt/signal/fwd.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <vector>

#include <SDL3/SDL_video.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "core/console_context.h"
#include "core/game_context.h"
#include "core/command_manager.h"
#include "core/command.h"
#include "platform/window.h"
#include "events/resize_event.h"
#include "utils/get_transform_matrix.h"
#include "components/text.h"
#include "components/text_mesh.h"
#include "components/mesh.h"
#include "components/transform.h"
#include "resources/shader.h"
#include "loaders/shader_loader.h"


Renderer::Renderer(entt::registry* registry, Window* window)
  : _pRegistry(registry),
    _pWindow(window)
{
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();
  dispatcher.sink<ResizeEvent>().connect<&Renderer::onResize>(this);
}


Renderer::~Renderer()
{
  _pRegistry->view<TextMesh>().each([this](TextMesh& mesh){
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ebo);
  });

  _pRegistry->view<Mesh>().each([this](Mesh& mesh){
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
  glClearColor(0.773f, 0.729f, 1.0f, 1.0f);

  registerCommands();

  _pTextShader = new Shader{ LoadShader("msdf_font") };
  _pUIShader = new Shader{ LoadShader("ui") };

  _ortho = glm::ortho(0.0f, (float)game_context.screenInfo.width, 0.0f, (float)game_context.screenInfo.height, -1.0f, 1.0f);

  return true;
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
  glDisable(GL_CULL_FACE);

  glUseProgram(_pTextShader->program);
  glUniformMatrix4fv(glGetUniformLocation(_pTextShader->program, "u_projection"), 1, GL_FALSE, &_ortho[0][0]);
  glUseProgram(_pUIShader->program);
  glUniformMatrix4fv(glGetUniformLocation(_pUIShader->program, "u_projection"), 1, GL_FALSE, &_ortho[0][0]);
  glUseProgram(0);
  
  _pRegistry->view<Text, TextMesh>().each([this](auto entity, Text& text, TextMesh& textMesh)
  {
    if (text.text.empty()) return;

    if (_pRegistry->all_of<Mesh, Transform>(entity))
    {
      glUseProgram(_pUIShader->program);
      auto model = GetTransformMatrix(_pRegistry->get<Transform>(entity));
      glUniformMatrix4fv(glGetUniformLocation(_pUIShader->program, "u_model"), 1, GL_FALSE, &model[0][0]);
      Mesh& mesh = _pRegistry->get<Mesh>(entity);
      glBindVertexArray(mesh.vao);
      glDrawElements(GL_TRIANGLES, mesh.indiceCount, GL_UNSIGNED_INT, nullptr);
      glUseProgram(0);
    }

    glUseProgram(_pTextShader->program);
    glBindTextureUnit(0, text.pFont->textureHandle);
    glUniform1f(glGetUniformLocation(_pTextShader->program, "pxRange"), text.pFont->pixelRange);
    glBindVertexArray(textMesh.vao);
    glDrawElements(GL_TRIANGLES, textMesh.indices.size(), GL_UNSIGNED_INT, nullptr);
    glUseProgram(0);
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