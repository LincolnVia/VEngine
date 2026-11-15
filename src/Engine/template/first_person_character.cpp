#include "box3d/math_functions.h"
#include "engine/physics/physics.hpp"
#include "raylib.h"
#include <box3d/box3d.h>
#include <box3d/id.h>
#include <box3d/types.h>
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
  interactionSystem.Init(world, *this, &cameraholder.camera);

  CreatePhysicsBody();

  cameraholder.camera.position = B3PosToVector3(getPosition());
  cameraholder.camera.target = {0.0f, 0.0f, 1.0f};
  cameraholder.camera.up = {0.0f, 1.0f, 0.0f};
  cameraholder.camera.fovy = 60.0f;
  cameraholder.camera.projection = CAMERA_PERSPECTIVE;

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
  cameraholder.rot.yaw = angle * DEG2RAD;
}

void FirstPersonCharacter::Update() {
  movement.HandlePlayerMovement(*_world, collider, cameraholder.rot);
  if (isFocused)
    cameraholder.Update(collider.bodyId);
  interactionSystem.Input();
}
void FirstPersonCharacter::Draw() {}

} // namespace engine
