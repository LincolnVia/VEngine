#include "engine/systems/interaction.hpp"
#include "engine/entities.hpp"
#include "engine/utils.hpp"
#include "engine/world/world.hpp"

#include "engine/template/first_person_character.hpp"
#include "raylib.h"

#include <chrono>
#include <cstdio>
#include <ctime>

namespace engine {

InteractionSystem::InteractionSystem() {}

void InteractionSystem::Init(World &_world, FirstPersonCharacter &_playerData,
                             Camera3D *_camera) {
  world = &_world;
  player = &_playerData;
  camera = _camera;
}
InteractionIntention InteractionSystem::GetIntention() {
  if (IsKeyPressed(KEY_E))

  {
    lastinteractionIntention = InteractionIntention::HOLD;
    return lastinteractionIntention;
  }
  if (IsKeyPressed(KEY_F)) {
    lastinteractionIntention = InteractionIntention::USE;
    return lastinteractionIntention;
  } else {
    lastinteractionIntention = InteractionIntention::NONE;
    return lastinteractionIntention;
  }
}

void InteractionSystem::SetHeldVariables() {
  ////// Preserve the cube's normal physics settings. While held, gravity
  // and low damping fight the hold joint and make the cube sag or
  // slingshot.
  heldObjectGravityScale = b3Body_GetGravityScale(heldObject->bodyId);
  heldObjectLinearDamping = b3Body_GetLinearDamping(heldObject->bodyId);
  heldObjectAngularDamping = b3Body_GetAngularDamping(heldObject->bodyId);

  b3Body_SetGravityScale(heldObject->bodyId, 0.0f);
  b3Body_SetLinearDamping(heldObject->bodyId, heldLinearDampingWhileHeld);
  b3Body_SetAngularDamping(heldObject->bodyId, heldAngularDampingWhileHeld);
  b3Body_SetLinearVelocity(heldObject->bodyId, b3Vec3_zero);
  b3Body_SetAngularVelocity(heldObject->bodyId, b3Vec3_zero);

  // This invisible kinematic body is the point the cube follows. Moving
  // a target body lets Box3D solve collisions instead of teleporting
  // the cube.
  b3BodyDef targetDef = b3DefaultBodyDef();
  targetDef.type = b3_kinematicBody;
  targetDef.position = b3ToPos(GetHoldPosition());
  targetDef.enableSleep = false;
  holdTargetBodyId = b3CreateBody(world->GetWorldId(), &targetDef);

  // Motor joint:
  // body A is the kinematic hold target, body B is the cube.
  // localFrameA/B identity means the cube center tries to match the
  // target body origin.
  b3MotorJointDef jointDef = b3DefaultMotorJointDef();
  jointDef.base.bodyIdA = holdTargetBodyId;
  jointDef.base.bodyIdB = heldObject->bodyId;
  jointDef.base.localFrameA = b3Transform_identity;
  jointDef.base.localFrameB = b3Transform_identity;

  // Hertz is spring stiffness: higher means snappier, but too high can
  // jitter. Damping > 1 is slightly overdamped so it returns without
  // bouncing around the hold point.
  jointDef.linearHertz = heldLinearHertz;
  jointDef.linearDampingRatio = heldLinearDampingRatio;
  jointDef.maxSpringForce = heldMaxSpringForce;

  // Angular spring keeps the cube from tumbling wildly while held.
  jointDef.angularHertz = heldAngularHertz;
  jointDef.angularDampingRatio = heldAngularDampingRatio;
  jointDef.maxSpringTorque = heldMaxSpringTorque;

  // Zero desired relative velocity with a force limit acts like a
  // brake: it removes flyaway energy without making the cube completely
  // rigid.
  jointDef.linearVelocity = b3Vec3_zero;
  jointDef.angularVelocity = b3Vec3_zero;
  jointDef.maxVelocityForce = heldMaxVelocityForce;
  jointDef.maxVelocityTorque = heldMaxVelocityTorque;

  holdJointId = b3CreateMotorJoint(world->GetWorldId(), &jointDef);
  b3Body_SetAwake(heldObject->bodyId, true);
}
void InteractionSystem::DropHeldObject() {
  printf("[Pickup] Dropping %s (body %i)\n", heldObject->name.c_str(),
         heldObject->bodyId.index1);
  // Dropping removes the constraint and helper body, then restores the
  // object's normal physics tuning. Velocity is kept, but capped so the
  // cube can be thrown without gaining runaway speed from the hold joint.
  if (B3_IS_NON_NULL(holdJointId)) {
    b3DestroyJoint(holdJointId, true);
    holdJointId = b3_nullJointId;
  }

  if (B3_IS_NON_NULL(holdTargetBodyId)) {
    b3DestroyBody(holdTargetBodyId);
    holdTargetBodyId = b3_nullBodyId;
  }

  b3Body_SetGravityScale(heldObject->bodyId, heldObjectGravityScale);
  b3Body_SetLinearDamping(heldObject->bodyId, heldObjectLinearDamping);
  b3Body_SetAngularDamping(heldObject->bodyId, heldObjectAngularDamping);
  b3Body_SetLinearVelocity(
      heldObject->bodyId,
      ClampVectorLength(b3Body_GetLinearVelocity(heldObject->bodyId),
                        heldReleaseMaxSpeed));
  b3Body_SetAngularVelocity(
      heldObject->bodyId,
      ClampVectorLength(b3Body_GetAngularVelocity(heldObject->bodyId),
                        heldReleaseMaxAngularSpeed));

  heldObject = nullptr;
}
void InteractionSystem::Input() {
  // Just drops and resets Stuff
  if (IsKeyPressed(KEY_E) && heldObject != nullptr) {
    DropHeldObject();
    return;
  }

  // Interactables / wallbuys dropped guns
  if (IsKeyPressed(KEY_F) && heldObject == nullptr) {
    RayFireResult ray =
        FireCameraSphereRay(*camera, *world, pickupDistance, pickupRadius,

                            CollisionCategory::Interactable);

    if (!ray.hit.hit) {
      printf("[Interaction] No hit (distance %.2f, radius %.2f)\n",
             pickupDistance, pickupRadius);
      return;
    }

    const b3ShapeId shapeId = ray.hit.shapeId;
    const b3BodyId bodyId = b3Shape_GetBody(shapeId);
    printf("[Interaction] Hit shape %i on body %i at (%.2f, %.2f, %.2f)\n",
           shapeId.index1, bodyId.index1, ray.hit.point.x, ray.hit.point.y,
           ray.hit.point.z);

    bool foundInteractable = false;
    for (const auto &entity : world->getRuntimeEntities()) {
      if (auto *interactable =
              dynamic_cast<info_interactable *>(entity.get())) {
        if (B3_ID_EQUALS(interactable->boxdef.bodyId, bodyId)) {
          interactable->Interact();
          foundInteractable = true;
          break;
        }
      }
    }

    if (!foundInteractable) {
      printf("[Interaction] Hit body %i is not a runtime interactable\n",
             bodyId.index1);
      return;
    }

    printf("[Interaction] Activated info_interactable (body %i)\n",
           bodyId.index1);
  }

  // Picks up The Body in raycast

  if (IsKeyPressed(KEY_E) && heldObject == nullptr && cooldownTimer <= 0.0f) {

    RayFireResult ray =
        FireCameraSphereRay(*camera, *world, pickupDistance, pickupRadius,

                            CollisionCategory::Dynamic);

    if (!ray.hit.hit) {
      printf("[Pickup] No hit (distance %.2f, radius %.2f)\n", pickupDistance,
             pickupRadius);
      return;
    }

    const b3ShapeId shapeId = ray.hit.shapeId;
    const b3BodyId bodyId = b3Shape_GetBody(shapeId);
    printf("[Pickup] Hit shape %i on body %i at (%.2f, %.2f, %.2f)\n",
           shapeId.index1, bodyId.index1, ray.hit.point.x, ray.hit.point.y,
           ray.hit.point.z);

    EntityData *hitBox = nullptr;
    for (const auto &entity : world->getRuntimeEntities()) {
      if (auto *dynamicBox = dynamic_cast<prop_dynamic *>(entity.get())) {
        if (B3_ID_EQUALS(dynamicBox->boxdef.bodyId, bodyId)) {
          hitBox = &dynamicBox->boxdef;
          break;
        }
      }
    }

    if (hitBox == nullptr) {
      printf("[Pickup] Hit body %i is not a runtime dynamic prop\n",
             bodyId.index1);
      return;
    }

    printf("[Pickup] Holding %s (body %i)\n", hitBox->name.c_str(),
           hitBox->bodyId.index1);
    setHeldObject(hitBox);
    SetHeldVariables();           // idk
    cooldownTimer = holdCooldown; // Reset the cooldown timer
    return;
  }
  UpdateHeldPosition();
  UpdateCooldownTimer();
}

// If the player is holding an object, update its position to stay in front
// of the player
void InteractionSystem::UpdateHeldPosition() {
  if (heldObject != nullptr) {
    if (heldObject != nullptr && B3_IS_NON_NULL(holdTargetBodyId)) {
      const b3Vec3 holdPosition = GetHoldPosition();

      // Drive the kinematic target toward the camera hold point each frame.
      // The motor joint then pulls the cube to that target through the
      // solver.
      b3Body_SetTargetTransform(holdTargetBodyId,
                                {b3ToPos(holdPosition), b3Quat_identity},
                                GetFrameTime(), true);
    }
  }
}
void InteractionSystem::UpdateCooldownTimer() {
  // Update the cooldown timer
  if (cooldownTimer > 0.0f && heldObject == nullptr) {
    cooldownTimer -= GetFrameTime();
  }
}

void InteractionSystem::Update(const Camera3D &camera) {}

EntityData *InteractionSystem::setHeldObject(EntityData *object) {
  return heldObject = object;
}

EntityData *InteractionSystem::getHeldObject() const { return heldObject; }

b3Vec3 InteractionSystem::GetHoldPosition() const {
  const b3CosSin pitchTrig = b3ComputeCosSin(player->getCameraRotation().pitch);

  const b3CosSin yawTrig = b3ComputeCosSin(player->getCameraRotation().yaw);

  const b3Vec3 forward = {
      pitchTrig.cosine * yawTrig.sine,
      pitchTrig.sine,
      pitchTrig.cosine * yawTrig.cosine,
  };

  const b3Vec3 eye =
      b3Add(b3ToVec3(b3Body_GetPosition(player->getEntityData().bodyId)),

            {0.0f, 1.0f, 0.0f});

  return b3Add(eye, b3MulSV(holdDistance, forward));
}

float *InteractionSystem::getHeldObjectCooldown() { return &cooldownTimer; }

void InteractionSystem::DisplayGrabPoint(Vector3 position) {
  DrawSphere(position, pickupRadius / 4, RED);
}

} // namespace engine
