#pragma once

#include "engine/graphics/graphics.hpp"
#include "engine/physics/physics.hpp"
#include "engine/systems/interaction.hpp"
#include "raylib.h"
namespace engine {

class World;

struct CameraRotation {
  float yaw = 0;
  float pitch = 0;
  float roll = 0;
};

class FirstPersonCharacter {
public:
  FirstPersonCharacter() = default;
  ~FirstPersonCharacter();

  void Init(World &world, bool captureCursor = true);
  void CreatePhysicsBody();

  void Input();
  void Update();
  void Draw();

  void HandlePlayerMovement();
  void HandleCameraLook();

  void Teleport(Vector3 position, float angle = 0);

  // Getter Setters for player variables
  b3Pos getPosition() const { return b3Body_GetPosition(collider.bodyId); }
  b3Vec3 getVelocity() const {
    return b3Body_GetLinearVelocity(collider.bodyId);
  }
  Camera3D &getCamera() { return camera; }
  const Camera3D &getCamera() const { return camera; }
  EntityData &getEntityData() { return collider; }
  CameraRotation getCameraRotation() { return {yaw, pitch, roll}; };

private:
  Camera3D camera{};
  World *_world = nullptr;
  EntityData collider;
  InteractionSystem interactionSystem;

  // Camera Variables
  float pitch = 0.0f;
  float yaw = 0.0f;
  float roll;
  float sensitivity = 0.0005f;

  // Defualt player move stats which get modifyed by the surface material
  float moveSpeed = 8.0f;
  float groundAccel = 15.0f;
  float airAccel = 3.0f;
  float groundFriction = 8.0f;
  float jumpSpeed = 5.0f;
  float jumpRayDistance = 3.5;

  float maxExternalHorizontalSpeed = 12.0f;
  float maxExternalUpSpeed = 7.0f;
  float maxExternalDownSpeed = 30.0f;

  bool isGrounded = false;
};

} // namespace engine
