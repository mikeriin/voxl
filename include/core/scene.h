#ifndef VOXL_SCENE_H
#define VOXL_SCENE_H


#include <string_view>

#include <entt/entt.hpp>

#include "events/dev_console_message_event.h"


class Scene
{
public:
  Scene(entt::registry* registry);
  ~Scene() = default;

  void DisplayGraph(bool* pOpen);
  
private:
  entt::registry* _pRegistry;
  
  entt::entity _selectedEntity;
  bool _isInspectorOpen;
  entt::entity _entityRenaming;
  char _renameBuffer[256] = "";
  bool _focusRenameInput;

  void displayInspector();
  void selectEntity(entt::entity e);

  template<typename Component>
  void addComponent(const Component& comp = {});
};


template<typename Component>
void Scene::addComponent(const Component& comp)
{
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

  _pRegistry->emplace_or_replace<Component>(_selectedEntity, comp);

  std::string_view type_name = entt::type_name<Component>::value();

  dispatcher.enqueue<DevConsoleMessageEvent>(DevConsoleMessageEvent{
    .level = DebugLevel::INFO,
    .buffer = "Added " + std::string(type_name) + " to entity[" + std::to_string((uint32_t) _selectedEntity) + "]"
  });
}


#endif // !VOXL_SCENE_H