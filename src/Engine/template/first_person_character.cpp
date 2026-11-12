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
  interactionSystem.Init(world, *this, &camera);

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
FirstPersonCharacter::~FirstPersonCharacter() {}

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
  interactionSystem.Input();
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

void FirstPersonCharacter::Draw() {}

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

} // namespace engine
