#ifndef VOXL_RENDERER_H
#define VOXL_RENDERER_H


#include <SDL3/SDL_video.h>
#include <entt/entity/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Window;

struct ResizeEvent;
struct Shader;


class Renderer
{
public:
  Renderer(entt::registry* registry, Window* window);
  ~Renderer();
  
  bool Init();
  void UpdateFont();

  void BeginFrame();
  void Render();
  void EndFrame();

private:
  entt::registry* _pRegistry;
  Window* _pWindow;
  SDL_GLContext _glCtx;

  Shader* _pTextShader;
  Shader* _pUIShader;

  glm::mat4 _ortho;

  void registerCommands();

  void onResize(const ResizeEvent& e);
};


#endif // !VOXL_RENDERER_H