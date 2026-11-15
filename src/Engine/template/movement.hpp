
#pragma once
#include "engine/template/camera.hpp"
#include "engine/utils.hpp"
#include <engine/world/world.hpp>
namespace engine {

struct MovementStats {
  float moveSpeed = 8.0f;
  float groundAccel = 15.0f;
  float airAccel = 3.0f;
  float groundFriction = 8.0f;
  float jumpSpeed = 5.0f;
  float jumpRayDistance = 3.5;

  float maxExternalHorizontalSpeed = 12.0f;
  float maxExternalUpSpeed = 7.0f;
  float maxExternalDownSpeed = 30.0f;
};

struct PlayerMovement {
  MovementStats moveStats;
  bool isGrounded = false;
  void HandlePlayerMovement(World &world, EntityData &entityData,
                            CameraRotation &rot) {

    float dt = GetFrameTime();

    const b3Vec3 bodyPos = b3ToVec3(b3Body_GetPosition(entityData.bodyId));

    RayFireResult groundRay =
        FireRay(bodyPos, {0, -1, 0}, world.GetWorldId(),
                moveStats.jumpRayDistance, CollisionCategory::World);

    isGrounded = groundRay.hit.hit;

    SurfaceMoveStats surfaceStats{};

    if (groundRay.hit.hit) {
      for (const auto &object : world.GetSceneObjects()) {
        if (B3_ID_EQUALS(object.shapeId, groundRay.hit.shapeId)) {
          surfaceStats = GetSurfaceMoveStats(object.material);
          break;
        }
      }
    }
    const float effectiveMoveSpeed =
        moveStats.moveSpeed * surfaceStats.speedMultiplier;

    const float effectiveGroundAccel =
        moveStats.groundAccel * surfaceStats.accelMultiplier;
    const float effectiveGroundFriction =
        moveStats.groundFriction * surfaceStats.frictionMultiplier;
    const float effectiveJumpSpeed =
        moveStats.jumpSpeed * surfaceStats.jumpMultiplier;

    const b3CosSin yawTrig = b3ComputeCosSin(rot.yaw);
    const b3Vec3 forward = {yawTrig.sine, 0.0f, yawTrig.cosine};
    const b3Vec3 right = {yawTrig.cosine, 0.0f, -yawTrig.sine};

    b3Vec3 wishMove = b3Vec3_zero;

    if (IsKeyDown(KEY_W))
      wishMove = b3Add(wishMove, forward);
    if (IsKeyDown(KEY_S))
      wishMove = b3Sub(wishMove, forward);
    if (IsKeyDown(KEY_A))
      wishMove = b3Add(wishMove, right);
    if (IsKeyDown(KEY_D))
      wishMove = b3Sub(wishMove, right);

    if (b3LengthSquared(wishMove) > 0.0f)
      wishMove = b3Normalize(wishMove);

    b3Vec3 vel = b3Body_GetLinearVelocity(entityData.bodyId);
    const bool wantsMove = b3LengthSquared(wishMove) > 0.0f;

    if (isGrounded) {
      ApplyFriction(vel, effectiveGroundFriction, dt);

      if (wantsMove)
        Accelerate(vel, wishMove, effectiveMoveSpeed, effectiveGroundAccel, dt);

      if (IsKeyPressed(KEY_SPACE)) {
        vel.y = effectiveJumpSpeed;
        isGrounded = false;
      }
    } else {
      if (wantsMove)
        Accelerate(vel, wishMove, moveStats.moveSpeed, moveStats.airAccel, dt);
    }

    const b3Vec3 horizontalVelocity = {vel.x, 0.0f, vel.z};
    const b3Vec3 clampedHorizontalVelocity = ClampVectorLength(
        horizontalVelocity, moveStats.maxExternalHorizontalSpeed);
    vel.x = clampedHorizontalVelocity.x;
    vel.z = clampedHorizontalVelocity.z;
    vel.y = b3ClampFloat(vel.y, -moveStats.maxExternalDownSpeed,
                         moveStats.maxExternalUpSpeed);

    b3Body_SetLinearVelocity(entityData.bodyId, vel);
  }
};
} // namespace engine
