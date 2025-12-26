#ifndef VOXL_SCENE_H
#define VOXL_SCENE_H


#include <entt/entt.hpp>


class Scene
{
public:
  Scene(entt::registry* registry);
  ~Scene() = default;

  void DisplayGraph();

private:
  entt::registry* _pRegistry;

  entt::entity _selectedEntity;
};


#endif // !VOXL_SCENE_H