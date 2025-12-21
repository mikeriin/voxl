#ifndef VOXL_UI_NODE_H
#define VOXL_UI_NODE_H


#include <vector>

#include <entt/entity/fwd.hpp>


struct UINode
{
  bool isDirty;
  entt::entity parent;
  std::vector<entt::entity> children;
};


#endif // !VOXL_UI_NODE_H