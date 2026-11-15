#pragma once

#include "engine/world/world.hpp"
#include <rlImGui.h>

class Debugger {
public:
  void Init();
  ~Debugger();

  engine::World *world = nullptr;

  void Update();
  void Render();
};
