#pragma once

#include <engine/physics/physics.hpp>
#include <engine/world/entity.hpp>
#include <utility>

namespace game {

struct wallbuy : engine::RuntimeEntity {
  engine::EnitityData boxdef;
  float modelScale = 1.0f;
  int cost = 0;
  int purchased = 0;
  int enabled = 1;

  wallbuy(engine::EnitityData object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~wallbuy() override { engine::DestroyRuntimeBody(boxdef); }
};

} // namespace game
