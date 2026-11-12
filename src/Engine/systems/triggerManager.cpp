#include "engine/systems/triggerManager.hpp"

#include "engine/entities.hpp"
#include "engine/world/world.hpp"

namespace engine {

void TriggerManager::Update() {
  for (auto &entity : world.getRuntimeEntities()) {
    if (auto *trigger = dynamic_cast<info_trigger *>(entity.get())) {
      trigger->Update(playerBody);
    }
  }
}

} // namespace engine
