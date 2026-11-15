#include "game/enemies/hound.hpp"
#include "zombie.hpp"

namespace game {

Hound::Hound(b3Vec3 position, b3Vec3 rotation, b3WorldId worldid)
    : position(position), rotation(rotation) {}

void Hound::Update() {}

void Hound::CheckPlayerDistance() {}

} // namespace game
