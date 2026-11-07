#pragma once

#include "box3d/id.h"
#include "box3d/math_functions.h"

namespace game {

class Hound {
  enum class STATE { IDLE, WALK, CHASE, DEAD };

  struct HoundStats {
    float speed;
    float health;
  };

public:
  Hound(const Hound &) = default;
  Hound(Hound &&) = default;
  Hound &operator=(const Hound &) = default;
  Hound &operator=(Hound &&) = default;
  Hound(b3Vec3 position, b3Vec3 rotation, b3WorldId worldid);
  void Update();
  void CheckPlayerDistance();

private:
  b3Vec3 position;
  b3Vec3 rotation;
  b3BodyId bodyid;
  HoundStats houndStats;
};

} // namespace game
