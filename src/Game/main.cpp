#include "engine/box3d_world.hpp"
#include "engine/template/first_person_character.hpp"
#include "raylib.h"
#include <engine/engine.hpp>

inline bool useMouseInput = true;

int main(int argc, char *argv[]) {
  engine::Engine engine("Box3D Game");
  engine::FirstPersonCharacter player;
  engine::Box3DWorld world(engine, player, useMouseInput);
  const char *bspPath = argc > 1 ? argv[1] : "resources/scenes/initial_box3d.bsp";
  world.LoadBspLevel(bspPath);

  while (!engine.ShouldClose()) {
    world.Update();
    if (useMouseInput)
      player.Input();
    player.Update();
    // BEFORE VISUAL CHANGE
    BeginDrawing();
    ClearBackground(SKYBLUE);

    BeginMode3D(player.getCamera());
    BeginShaderMode(engine.getShaderManager().getShader("lighting"));
    world.GetWorld().Draw();
    player.Draw();
    EndShaderMode();
    EndMode3D();

    EndDrawing();
  }
  return 0;
}
