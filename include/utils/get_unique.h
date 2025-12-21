#ifndef VOXL_GET_UNIQUE_H
#define VOXL_GET_UNIQUE_H


#include <type_traits>

#include <entt/entt.hpp>


template<typename T>
concept IsTag = std::is_empty_v<T>;


template<IsTag T>
inline entt::entity GetUnique(entt::registry& registry)
{
  auto view = registry.view<T>();
  return view.empty() ? entt::null: view.front();
}


#endif // !VOXL_GET_UNIQUE_H