#pragma once
#include "Signal.h"
#include "box3d/box3d.h"
#include "box3d/id.h"
#include "engine/graphics/graphics.hpp"
#include "engine/physics/physics.hpp"
#include "engine/template/first_person_character.hpp"
#include "raylib.h"
#include "world/entity.hpp"
#include <cstdio>
namespace engine {

struct prop_dynamic : engine::RuntimeEntity {
  engine::EntityData boxdef;
  float modelScale = 1.0f;

  prop_dynamic(engine::EntityData object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~prop_dynamic() override { engine::DestroyRuntimeBody(boxdef); }
};
enum class INTERACTION_TYPE {
  BASE,
  DOOR,
  WALLBUY,
};

struct info_interactable : public engine::RuntimeEntity {
  EntityData boxdef;
  Gallant::Signal1<int> OnInteracted;
  INTERACTION_TYPE interactable_type = INTERACTION_TYPE::BASE;
  float modelScale = 1.0f;

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
  info_interactable(Vector3 center, Vector3 halfExtents, b3WorldId &worldId,
                    float friction, float density) {
    boxdef = createDynamicObject(
        {center.x, center.y, center.z},
        {halfExtents.x, halfExtents.y, halfExtents.z}, density, friction,
        worldId,
        {CollisionCategory::Static | CollisionCategory::Interactable,
         CollisionCategory::Player | CollisionCategory::World, -1},
        SurfaceMaterial::Default);
  }
  ~info_interactable() override { engine::DestroyRuntimeBody(boxdef); }
  void Update(const b3BodyId &playerId) override {}
  virtual void OnInteract();
  void Interact() override {
    if (!boxdef.enabled)
      return;
    OnInteracted.Emit(target);
    TraceLog(LOG_INFO, " %i Interactaed with (target %d)", interactable_type,
             target);
    printf("interact %d \n", target);
  }
};
struct info_door : engine::info_interactable {

  int cost = 500;
  float openAngle = 0;
  info_door(Vector3 center, Vector3 halfExtents, b3WorldId &world)
      : engine::info_interactable(center, halfExtents, world) {
    interactable_type = INTERACTION_TYPE::DOOR;
  }
  ~info_door() override { engine::DestroyRuntimeBody(boxdef); }
  void Interact(FirstPersonCharacter &player) override {
    if (boxdef.enabled && !triggered) {

      triggered = 1;
      player.AddPoints(-cost);
      printf("Door Triggered\n");
      boxdef.enabled = false;
      // DestroyRuntimeBody(boxdef);
    }
  }
};

struct info_trigger : public engine::RuntimeEntity {
  EntityData entityData;
  int target = -1;
  int triggered = 0;

  info_trigger(Vector3 center, Vector3 halfExtents, b3WorldId &worldId) {
    entityData = createStaticObject(
        {center.x, center.y, center.z},
        {halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f},
        worldId, {CollisionCategory::Trigger, CollisionCategory::Player, -1},
        SurfaceMaterial::Default, true);
  }
  ~info_trigger() override { engine::DestroyRuntimeBody(entityData); }
  void on_trigger_collided() {
    if (!entityData.enabled || triggered)
      return;
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
