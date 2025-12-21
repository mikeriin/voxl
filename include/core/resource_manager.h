#ifndef VOXL_RESOURCE_MANAGER_H
#define VOXL_RESOURCE_MANAGER_H


#include <unordered_map>
#include <any>
#include <string>
#include <utility>

#include <entt/entt.hpp>

#include "resources/traits.h"


class ResourceManager
{
public:
  ResourceManager() = default;
  ~ResourceManager() = default;

  template<typename Resource, typename... Args>
  auto Load(const std::string& name, Args&&... args);

  template<typename Resource>
  entt::resource<Resource> Get(const std::string& name);

  // les *ByID c'est pour utiliser "id_name"_hs qui est plus rapide
  template<typename Resource, typename... Args>
  auto LoadByID(entt::id_type id, Args&&... args);

  template<typename Resource>
  entt::resource<Resource> GetByID(entt::id_type id);

private:
  std::unordered_map<entt::id_type, std::any> _caches;

  template<typename Resource, typename Loader>
  entt::resource_cache<Resource, Loader>& getCacheInternal(); 
};


template<typename Resource, typename... Args>
inline auto ResourceManager::Load(const std::string& name, Args&&... args)
{
  const auto id = entt::hashed_string(name.c_str());
  return LoadByID<Resource>(id, std::forward<Args>(args)...);
}


template<typename Resource>
inline entt::resource<Resource> ResourceManager::Get(const std::string& name)
{
  const auto id = entt::hashed_string(name.c_str());
  return GetByID<Resource>(id);
}


template<typename Resource, typename... Args>
inline auto ResourceManager::LoadByID(entt::id_type id, Args&&... args)
{
  using Loader = typename ResourceTraits<Resource>::Loader;
  return getCacheInternal<Resource, Loader>().load(id, std::forward<Args>(args)...);
}


template<typename Resource>
inline entt::resource<Resource> ResourceManager::GetByID(entt::id_type id)
{
  using Loader = typename ResourceTraits<Resource>::Loader;
  return getCacheInternal<Resource, Loader>()[id];
}


template<typename Resource, typename Loader>
inline entt::resource_cache<Resource, Loader>& ResourceManager::getCacheInternal()
{
  const auto id = entt::type_id<Resource>().hash();
  if (_caches.find(id) == _caches.end()) 
  {
    _caches[id] = entt::resource_cache<Resource, Loader>{};
  }
  return std::any_cast<entt::resource_cache<Resource, Loader>&>(_caches[id]);
}


#endif // !VOXL_RESOURCE_MANAGER_H