#pragma once
#include "Signal.h"
#include "box3d/box3d.h"
#include "box3d/id.h"
#include "engine/graphics/graphics.hpp"
#include "engine/physics/physics.hpp"
#include "raylib.h"
#include "world/entity.hpp"
#include <cstdio>
namespace engine {

struct prop_dynamic : engine::RuntimeEntity {
  engine::EntityData boxdef;
  float modelScale = 1.0f;
  int enabled = 1;

  prop_dynamic(engine::EntityData object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~prop_dynamic() override { engine::DestroyRuntimeBody(boxdef); }
};
struct info_interactable : public engine::RuntimeEntity {
  EntityData boxdef;
  Gallant::Signal1<int> OnInteracted;
  float modelScale = 1.0f;
  int enabled = 1;
  int target = -1;
  int triggered = 0;
  info_interactable(Vector3 center, Vector3 halfExtents, b3WorldId &worldId) {
    boxdef = createStaticObject(
        {center.x, center.y, center.z},
        {halfExtents.x, halfExtents.y, halfExtents.z}, worldId,
        {CollisionCategory::Static | CollisionCategory::Interactable,
         CollisionCategory::Player | CollisionCategory::World, -1},
        SurfaceMaterial::Default);
  }
  ~info_interactable() override { engine::DestroyRuntimeBody(boxdef); }
  void Update(const b3BodyId &playerId) override {}

  void Interact() override {
    if (!enabled)
      return;
    OnInteracted.Emit(target);
    printf("Interacted with info_interactable");
  }
};

struct info_trigger : public engine::RuntimeEntity {
  EntityData entityData;
  Gallant::Signal1<int> OnTriggered;
  int target = -1;
  int triggered = 0;
  int enabled = 1;
  info_trigger(Vector3 center, Vector3 halfExtents, b3WorldId &worldId) {
    entityData = createStaticObject(
        {center.x, center.y, center.z},
        {halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f},
        worldId, {CollisionCategory::Trigger, CollisionCategory::Player, -1},
        SurfaceMaterial::Default, true);
  }
  ~info_trigger() override { engine::DestroyRuntimeBody(entityData); }
  void on_trigger_collided() {
    if (enabled)
      triggered = 1;
    OnTriggered.Emit(target);
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

      if (B3_ID_EQUALS(b3Shape_GetBody(visitorId), playerId)) {
        if (!triggered) {
          Interact();
          TraceLog(LOG_INFO, "Trigger fired (target %d)", target);
        }
      }
    }
  }
};

} // namespace engine
