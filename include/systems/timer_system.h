#ifndef VOXL_TIMER_SYSTEM_H
#define VOXL_TIMER_SYSTEM_H


#include <entt/entt.hpp>
#include <iostream>

#include "components/timer.h"


struct TimerSystem
{
  void Update(entt::registry& registry, double dt)
  {
    registry.view<Timer>().each([&registry, dt](const auto& entity, auto& timer){
      if (timer.isActive) timer.time -= dt;

      if (timer.time <= 0.0)
      {
        registry.destroy(entity);
        std::cout << "[TimerSystem] entity destroyed\n";
      }
    });
  }
};


#endif // !VOXL_TIMER_SYSTEM_H