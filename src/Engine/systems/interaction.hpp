#pragma once

#include "engine/entities.hpp"
#include "engine/global.hpp"
#include "engine/graphics/graphics.hpp"
#include "engine/utils.hpp"
#include "engine/world/entityRegistry.hpp"
#include "engine/world/world.hpp"
#include "game/world/entities.hpp"
#include <box3d/box3d.h>
#include <cstdio>

namespace engine {

class InteractionSystem {
public:
  void Init(World &world) { _world = &world; }

  void Update(const Camera3D &camera) {
    if (_world == nullptr)
      return;

    RayFireResult ray =
        FireCameraRay(camera, *_world, 100.0f,
                      CollisionCategory::World | CollisionCategory::Trigger |
                          CollisionCategory::Dynamic);

    DrawRayDebug(ray.origin, ray.translation);

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      return;

    if (!ray.hit.hit) {
      printf("No hit.\n");
      return;
    }

    const b3ShapeId shapeId = ray.hit.shapeId;
    const b3BodyId bodyId = b3Shape_GetBody(shapeId);
    _world->Interact(bodyId);

    const b3Vec3 hitPoint = b3ToVec3(ray.hit.point);
    if (debugMode) {
      printf("Hit object!\n");
      printf("Hit Point: %.2f %.2f %.2f\n", hitPoint.x, hitPoint.y, hitPoint.z);
      printf("Normal: %.2f %.2f %.2f\n", ray.hit.normal.x, ray.hit.normal.y,
             ray.hit.normal.z);
    }

    for (const auto &entity : _world->getRuntimeEntities()) {
      const EnitityData *object = nullptr;

      if (const auto *interactable =
              dynamic_cast<const info_interactable *>(entity.get())) {
        object = &interactable->boxdef;
      }

      if (B3_ID_EQUALS(object->bodyId, bodyId) &&
          object->name == "info_interactable") {
        printf("Hit Interactable\n");
        break;
      }
    }
  }

private:
  World *_world = nullptr;
};

} // namespace engine
