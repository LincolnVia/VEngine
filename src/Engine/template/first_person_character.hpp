#pragma once

#include "box3d/id.h"
#include "camera.hpp"
#include "engine/physics/physics.hpp"
#include "engine/systems/interaction.hpp"
#include "engine/template/movement.hpp"
#include "raylib.h"
namespace engine {

class World;
struct GameStats {
  int points = 1000;
  int kills = 0;
  void Draw() {
    DrawText(TextFormat("Point: %i", points), 10, 100, 16, BLACK);
    DrawText(TextFormat("Kill: %i", kills), 10, 132, 16, BLACK);
  }
};
class FirstPersonCharacter {
public:
  FirstPersonCharacter() : interactionSystem(*this) {};

  ~FirstPersonCharacter();

  void Init(World &world, bool captureCursor = true);
  void CreatePhysicsBody();

  void Update();
  void Draw();

  void Teleport(Vector3 position, float angle = 0);

  // Getter Setters for player variables
  b3Pos getPosition() const { return b3Body_GetPosition(collider.bodyId); }
  b3Vec3 getVelocity() const {
    return b3Body_GetLinearVelocity(collider.bodyId);
  }
  Camera3D &getCamera() { return cameraholder.camera; }
  const Camera3D &getCamera() const { return cameraholder.camera; }
  EntityData &getEntityData() { return collider; }
  CameraRotation getCameraRotation() {
    return {cameraholder.rot.yaw, cameraholder.rot.pitch,
            cameraholder.rot.roll};
  };
  void AddPoints(int value) { stats.points += value; }
  int getPoints() { return stats.points; }
  bool getFocus() const { return isFocused; }
  void setFocus(bool val) { isFocused = val; }
  GameStats getStats() const { return stats; }
  void setStats(GameStats val) { stats = val; }

private:
  bool isFocused = true;
  PlayerCamera cameraholder;
  World *_world = nullptr;
  EntityData collider;
  InteractionSystem interactionSystem;

  GameStats stats;
  PlayerMovement movement;
  // Camera Variables

  // Defualt player move stats which get modifyed by the surface material
};

} // namespace engine
