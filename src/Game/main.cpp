#include "engine/box3d_world.hpp"
#include "engine/template/first_person_character.hpp"
#include "engine/ui/2dscene.hpp"
#include "game/world/zombiesWorld.hpp"
#include "raylib.h"
#include <engine/engine.hpp>
#include <filesystem>
#include <system_error>

inline bool useMouseInput = true;

namespace {
void SetProjectWorkingDirectory(const char *executablePath) {
  std::error_code error;
  std::filesystem::path candidate =
      std::filesystem::absolute(executablePath, error).parent_path();
  for (int depth = 0; depth < 4 && !candidate.empty(); ++depth) {
    if (std::filesystem::is_directory(candidate / "resources", error)) {
      std::filesystem::current_path(candidate, error);
      return;
    }
    candidate = candidate.parent_path();
  }
}
} // namespace

int main(int argc, char *argv[]) {
  SetProjectWorkingDirectory(argv[0]);
  engine::Engine engine("Box3D Game");
  engine::FirstPersonCharacter player;
  engine::Scene2D MainMenu;
  game::ZombiesWorld world(engine, player, useMouseInput);
  const char *bspPath =
      argc > 1 ? argv[1] : "resources/scenes/initial_box3d.bsp";
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
