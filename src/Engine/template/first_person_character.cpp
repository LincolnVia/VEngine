#include "box3d/math_functions.h"
#include "engine/entities.hpp"
#include "engine/physics/physics.hpp"
#include "engine/utils.hpp"
#include "engine/world/entityRegistry.hpp"
#include "game/world/entities.hpp"
#include "raylib.h"
#include <box3d/box3d.h>
#include <box3d/id.h>
#include <box3d/types.h>
#include <cstdio>
#include <cstring>
#include <engine/global.hpp>
#include <engine/graphics/graphics.hpp>
#include <engine/template/first_person_character.hpp>
#include <raylib.h>

namespace engine {
using engine::debugMode;

void FirstPersonCharacter::CreatePhysicsBody() {
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.type = b3_dynamicBody;
  bodyDef.position = b3ToPos({0.0f, 2.0f, 0.0f});
  bodyDef.motionLocks.angularX = true;
  bodyDef.motionLocks.angularZ = true;

  bodyDef.motionLocks.angularY = false;
  collider.bodyId = b3CreateBody(_world->GetWorldId(), &bodyDef);

  collider.halfExtents = {1.0f, 2.5f, 1.0f};
  collider.hull = b3MakeBoxHull(collider.halfExtents.x, collider.halfExtents.y,
                                collider.halfExtents.z);

  b3ShapeDef shapeDef = b3DefaultShapeDef();
  shapeDef.density = 985.0f;
  shapeDef.baseMaterial.friction = 0.46f;
  shapeDef.filter.categoryBits = CollisionCategory::Player;
  shapeDef.filter.maskBits =
      CollisionCategory::World | CollisionCategory::Trigger;
  shapeDef.enableSensorEvents = true;
  collider.name = "Player";
  collider.shapeId =
      b3CreateHullShape(collider.bodyId, &shapeDef, &collider.hull.base);

  globalPlayerShapeId = collider.shapeId;
}

void FirstPersonCharacter::Init(World &world, bool captureCursor) {
  _world = &world;
  interactionSystem.Init(world);

  CreatePhysicsBody();

  camera.position = B3PosToVector3(getPosition());
  camera.target = {0.0f, 0.0f, 1.0f};
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  if (captureCursor) {
    DisableCursor();
  } else {
    EnableCursor();
  }
}
FirstPersonCharacter::~FirstPersonCharacter() { UnloadModel(gunModel); }

void FirstPersonCharacter::Teleport(Vector3 position, float angle) {
  b3Body_SetTransform(collider.bodyId,
                      b3ToPos({position.x, position.y, position.z}),
                      b3Quat_identity);
  b3Body_SetLinearVelocity(collider.bodyId, b3Vec3_zero);
  b3Body_SetAngularVelocity(collider.bodyId, b3Vec3_zero);
  yaw = angle * DEG2RAD;
}

void FirstPersonCharacter::Input() {
  HandleCameraLook();
  HandlePlayerMovement();
  interactionSystem.Update(camera);
  PickupPhysicsObject();
}

void FirstPersonCharacter::Update() {

  const b3Vec3 bodyPosition = b3ToVec3(b3Body_GetPosition(collider.bodyId));
  const b3Vec3 cameraPosition = b3Add(bodyPosition, {0.0f, 1.0f, 0.0f});

  const b3CosSin pitchTrig = b3ComputeCosSin(pitch);
  const b3CosSin yawTrig = b3ComputeCosSin(yaw);
  const b3Vec3 view = {pitchTrig.cosine * yawTrig.sine, pitchTrig.sine,
                       pitchTrig.cosine * yawTrig.cosine};

  camera.position = B3Vec3ToVector3(cameraPosition);
  camera.target = B3Vec3ToVector3(b3Add(cameraPosition, view));
}

void FirstPersonCharacter::Draw() { DisplayGrabPoint(); }

void FirstPersonCharacter::HandleCameraLook() {
  Vector2 mouseDelta = GetMouseDelta();

  yaw -= mouseDelta.x * sensitivity;
  pitch -= mouseDelta.y * sensitivity;

  const float maxPitch = 89.0f * B3_DEG_TO_RAD;
  pitch = b3ClampFloat(pitch, -maxPitch, maxPitch);
}

void FirstPersonCharacter::HandlePlayerMovement() {
  float dt = GetFrameTime();

  const b3Vec3 bodyPos = b3ToVec3(b3Body_GetPosition(collider.bodyId));

  RayFireResult groundRay = FireRay(bodyPos, {0, -1, 0}, *_world,
                                    jumpRayDistance, CollisionCategory::World);
  isGrounded = groundRay.hit.hit;

  SurfaceMoveStats surfaceStats{};

  if (groundRay.hit.hit) {
    for (const auto &object : _world->GetSceneObjects()) {
      if (B3_ID_EQUALS(object.shapeId, groundRay.hit.shapeId)) {
        surfaceStats = GetSurfaceMoveStats(object.material);
        break;
      }
    }
  }
  const float effectiveMoveSpeed = moveSpeed * surfaceStats.speedMultiplier;
  const float effectiveGroundAccel = groundAccel * surfaceStats.accelMultiplier;
  const float effectiveGroundFriction =
      groundFriction * surfaceStats.frictionMultiplier;
  const float effectiveJumpSpeed = jumpSpeed * surfaceStats.jumpMultiplier;

  const b3CosSin yawTrig = b3ComputeCosSin(yaw);
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

  b3Vec3 vel = b3Body_GetLinearVelocity(collider.bodyId);
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
      Accelerate(vel, wishMove, moveSpeed, airAccel, dt);
  }

  const b3Vec3 horizontalVelocity = {vel.x, 0.0f, vel.z};
  const b3Vec3 clampedHorizontalVelocity =
      ClampVectorLength(horizontalVelocity, maxExternalHorizontalSpeed);
  vel.x = clampedHorizontalVelocity.x;
  vel.z = clampedHorizontalVelocity.z;
  vel.y = b3ClampFloat(vel.y, -maxExternalDownSpeed, maxExternalUpSpeed);

  b3Body_SetLinearVelocity(collider.bodyId, vel);
}

void FirstPersonCharacter::PickupPhysicsObject() {
  if (IsKeyPressed(KEY_E) && heldObject != nullptr) {
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
    return;
  }
  if (IsKeyPressed(KEY_E) && heldObject == nullptr && cooldownTimer <= 0.0f) {

    RayFireResult ray =
        FireCameraSphereRay(camera, *_world, pickupDistance, pickupRadius,
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

    EnitityData *hitBox = nullptr;
    for (const auto &entity : _world->getRuntimeEntities()) {
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

    // Preserve the cube's normal physics settings. While held, gravity
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
    holdTargetBodyId = b3CreateBody(_world->GetWorldId(), &targetDef);

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

    holdJointId = b3CreateMotorJoint(_world->GetWorldId(), &jointDef);
    b3Body_SetAwake(heldObject->bodyId, true);

    cooldownTimer = holdCooldown; // Reset the cooldown timer
    return;
  }

  // If the player is holding an object, update its position to stay in front of
  // the player
  if (heldObject != nullptr) {
    if (heldObject != nullptr && B3_IS_NON_NULL(holdTargetBodyId)) {
      const b3Vec3 holdPosition = GetHoldPosition();

      // Drive the kinematic target toward the camera hold point each frame.
      // The motor joint then pulls the cube to that target through the solver.
      b3Body_SetTargetTransform(holdTargetBodyId,
                                {b3ToPos(holdPosition), b3Quat_identity},
                                GetFrameTime(), true);
    }
  }
  // Update the cooldown timer
  if (cooldownTimer > 0.0f && heldObject == nullptr) {
    cooldownTimer -= GetFrameTime();
  }
}

} // namespace engine
