#ifndef INTERACTIONSYSTEM
#define INTERACTIONSYSTEM

#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "engine/physics/physics.hpp"
#include "raylib.h"
#include <box3d/box3d.h>
namespace engine {

class World;
class FirstPersonCharacter;

enum class InteractionIntention {
  NONE,
  HOLD,
  USE,
  DROP,

};
class InteractionSystem {
public:
  InteractionSystem();
  void Init(World &_world, FirstPersonCharacter &_playerData,
            Camera3D *_camera);
  void Input();
  void Update(const Camera3D &camera);
  EntityData *setHeldObject(EntityData *object);
  EntityData *getHeldObject() const;
  b3Vec3 GetHoldPosition() const;
  float *getHeldObjectCooldown();
  void DisplayGrabPoint(Vector3 position);
  InteractionIntention GetIntention();
  void ResetHeldObject();
  void SetHeldVariables();
  void UpdateHeldPosition();
  void UpdateCooldownTimer();
  void DropHeldObject();

private:
  World *world = nullptr;
  EntityData *heldObject = nullptr;
  FirstPersonCharacter *player = nullptr;
  InteractionIntention lastinteractionIntention = InteractionIntention::NONE;
  Camera3D *camera = nullptr;
  float pitch;
  float yaw;
  float heldObjectGravityScale = 1.0f;
  float heldObjectLinearDamping = 0.0f;
  float heldObjectAngularDamping = 0.0f;

  b3BodyId holdTargetBodyId = b3_nullBodyId;
  b3JointId holdJointId = b3_nullJointId;
  // Physics object pickup variables.
  float holdDistance = 5.0f;
  float pickupDistance = 4.0f;
  float pickupRadius = 0.55f;
  float holdCooldown = 0.5f;
  float cooldownTimer = 0.0f;
  b3Vec3 holdOffset = {0.0f, 1.6f, 3.0f};

  /// @name Held object restraint tuning
  /// Values for the motor joint that pulls the cube to the invisible kinematic
  /// hold target in front of the camera.
  /// @{
  float heldLinearHertz = 6.0f; ///< Position spring stiffness. Higher snaps to
                                ///< the hold point faster.
  float heldLinearDampingRatio =
      1.15f; ///< Position spring damping. 1 is critical damping; above 1
             ///< reduces bounce.
  float heldMaxSpringForce = 50000.0f; ///< Strongest force the position spring
                                       ///< can apply before giving up.
  float heldAngularHertz = 10.0f; ///< Rotation spring stiffness. Higher stops
                                  ///< held-object spinning faster.
  float heldAngularDampingRatio =
      1.1f; ///< Rotation spring damping. Above 1 keeps rotation from wobbling.
  float heldMaxSpringTorque =
      25000.0f; ///< Strongest torque the rotation spring can apply.
  float heldMaxVelocityForce =
      5000.0f; ///< Linear velocity brake strength for killing flyaway movement.
  float heldMaxVelocityTorque =
      8000.0f; ///< Angular velocity brake strength for killing flyaway spin.
  float heldLinearDampingWhileHeld =
      2.0f; ///< Extra body drag while held, restored when dropped.
  float heldAngularDampingWhileHeld =
      6.0f; ///< Extra spin drag while held, restored when dropped.
  float heldReleaseMaxSpeed =
      6.0f; ///< Maximum linear speed kept when released. Higher allows
            ///< harder throws; lower makes throws more consistent.
  float heldReleaseMaxAngularSpeed =
      18.0f; ///< Maximum spin speed kept when released. Higher allows more
             ///< tumbling; lower keeps thrown cubes calmer.
  /// @}
};

} // namespace engine
#endif // INTERACTIONSYSTEM
