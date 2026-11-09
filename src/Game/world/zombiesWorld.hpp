#pragma once

#include "engine/box3d_world.hpp"
#include "engine/template/first_person_character.hpp"
#include <cstdio>
namespace game {

class ZombiesWorld : public engine::Box3DWorld {
public:
  ZombiesWorld(engine::Engine &engine, engine::FirstPersonCharacter &player,
               bool captureCursor)
      : engine::Box3DWorld(engine, player, captureCursor) {
    printf("Zombie World Starting");
  }

  void Update() { ::engine::Box3DWorld::Update(); }

private:
};
} // namespace game
