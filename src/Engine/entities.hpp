#pragma once
#include "box3d/id.h"
#include "engine/physics/physics.hpp"
#include "world/entity.hpp"
#include <cstdio>
namespace engine {

struct prop_dynamic : engine::RuntimeEntity {
  engine::EnitityData boxdef;
  float modelScale = 1.0f;
  int enabled = 1;

  prop_dynamic(engine::EnitityData object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~prop_dynamic() override { engine::DestroyRuntimeBody(boxdef); }
};
struct info_interactable : public engine::RuntimeEntity {
  engine::EnitityData boxdef;
  int enabled = 1;
  int target = -1;
  int triggered = 0;
  info_interactable(engine::EnitityData object) : boxdef(std::move(object)) {}
  ~info_interactable() override { engine::DestroyRuntimeBody(boxdef); }
  void Interact() override { printf("Interacted with info_interactable"); }
};

struct info_trigger : public engine::RuntimeEntity {
  EnitityData entityData;
  int target = -1;
  int triggered = 0;
  int enabled = 1;
  info_trigger(Vector3 center, Vector3 halfExtents, b3WorldId &worldId) {
    entityData = createStaticObject(
        {center.x, center.y, center.z},
        {halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f},
        worldId, SurfaceMaterial::Default, true);
  }
  ~info_trigger() override { engine::DestroyRuntimeBody(entityData); }
  void on_trigger_collided() {
    if (enabled)
      triggered = 1;
  }
  void Interact() override { on_trigger_collided(); }
  void Update(const b3BodyId &playerId) override {
    // First determine the required array capacity to hold all the overlapping
    // shape ids.
    int capacity = b3Shape_GetSensorCapacity(entityData.shapeId);
    std::vector<b3ShapeId> overlaps(static_cast<size_t>(capacity));

    // Now get all overlaps and record the actual count
    int count =
        b3Shape_GetSensorData(entityData.shapeId, overlaps.data(), capacity);

    for (int i = 0; i < count; ++i) {
      b3ShapeId visitorId = overlaps[static_cast<size_t>(i)];

      // Ensure the visitorId is valid
      if (b3Shape_IsValid(visitorId) == false) {
        continue;
      }

      // process overlap using game logic
      if (B3_IS_NULL(playerId))
        return;

      if (B3_ID_EQUALS(visitorId, playerId)) {
        if (!triggered) {
          triggered = true;
          TraceLog(LOG_INFO, "Trigger fired (target %d)", target);
        }
      }
    }
  }
};

} // namespace engine
