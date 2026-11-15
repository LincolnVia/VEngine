#pragma once

#include <box3d/box3d.h>
#include <raylib.h>

namespace engine {

struct cubeBrush {
  b3WorldTransform transform{};
  b3Vec3 scale{};
  Color color{};
};

} // namespace engine
