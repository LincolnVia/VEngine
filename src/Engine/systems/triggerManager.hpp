#pragma once

#include <box3d/id.h>
namespace engine {
class World;

class TriggerManager {
public:
  TriggerManager(World &world, b3BodyId playerBody)
      : world(world), playerBody(playerBody) {}
  void Update();

private:
  World &world;
  b3BodyId playerBody;
};
} // namespace engine
