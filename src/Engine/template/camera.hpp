#pragma once

#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "engine/graphics/graphics.hpp"
#include "raylib.h"
namespace engine {

struct CameraRotation {
  float yaw = 0;
  float pitch = 0;
  float roll = 0;
};

struct PlayerCamera {
  Camera3D camera;
  CameraRotation rot;

  float sensitivity = 0.0005f;
  void Update(b3BodyId &bodyId) {
    UpdateLook();
    const b3Vec3 bodyPosition = b3ToVec3(b3Body_GetPosition(bodyId));
    const b3Vec3 cameraPosition = b3Add(bodyPosition, {0.0f, 1.0f, 0.0f});

    const b3CosSin pitchTrig = b3ComputeCosSin(rot.pitch);
    const b3CosSin yawTrig = b3ComputeCosSin(rot.yaw);
    const b3Vec3 view = {pitchTrig.cosine * yawTrig.sine, pitchTrig.sine,
                         pitchTrig.cosine * yawTrig.cosine};

    camera.position = B3Vec3ToVector3(cameraPosition);
    camera.target = B3Vec3ToVector3(b3Add(cameraPosition, view));
  }
  void UpdateLook()

  {
    Vector2 mouseDelta = GetMouseDelta();

    rot.yaw -= mouseDelta.x * sensitivity;

    rot.pitch -= mouseDelta.y * sensitivity;

    const float maxPitch = 89.0f * B3_DEG_TO_RAD;
    rot.pitch = b3ClampFloat(rot.pitch, -maxPitch, maxPitch);
  }
};

} // namespace engine
