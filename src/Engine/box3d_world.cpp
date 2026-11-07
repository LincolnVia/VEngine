#include "engine/template/first_person_character.hpp"
#include <engine/box3d_world.hpp>
#include <raylib.h>

namespace engine {
Box3DWorld::Box3DWorld(Engine &engine, FirstPersonCharacter &_player,
                       bool captureCursor)
    : engine(engine), world(engine), player(_player),
      enablePlayerInput(captureCursor) {
  engine.getShaderManager().Init();
  engine.getAssetManager().Init();
  world.Init();
  player.Init(world, captureCursor);
}

bool Box3DWorld::LoadBspLevel(const std::filesystem::path &path) {
  if (!world.LoadBspLevel(path))
    return false;
  if (world.HasBspPlayerSpawn()) {
    player.Teleport(world.GetBspPlayerSpawn().position,
                    world.GetBspPlayerSpawn().rotation);
  }
  return true;
}

void Box3DWorld::Update() { world.Update(); }

} // namespace engine
