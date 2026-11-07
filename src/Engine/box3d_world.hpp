#pragma once

#include "engine/template/first_person_character.hpp"
#include "engine/world/world.hpp"
#include <engine/engine.hpp>
namespace engine {

// Owns Box3D gameplay state and coordinates the player with the game world.
class Box3DWorld {
public:
  explicit Box3DWorld(Engine &engine, FirstPersonCharacter &player,
                      bool captureCursor = true);
  bool LoadBspLevel(const std::filesystem::path &path);
  void Update();
  World &GetWorld() { return world; }

private:
  Engine &engine;
  World world;
  FirstPersonCharacter &player;
  bool enablePlayerInput;
};

} // namespace engine
