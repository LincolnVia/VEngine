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

  Vector3 lightDir = Vector3Normalize((Vector3){1.0f, 3.0f, 1.0f});
  int lightDirLoc =
      GetShaderLocation(engine.getShaderManager().getShader("cel"), "lightDir");
  SetShaderValue(engine.getShaderManager().getShader("cel"), lightDirLoc,
                 &lightDir, SHADER_UNIFORM_VEC3);

  Vector2 renderSize = {(float)GetRenderWidth(), (float)GetRenderHeight()};
  int renderSizeLoc = GetShaderLocation(
      engine.getShaderManager().getShader("post_outline"), "renderSize");
  SetShaderValue(engine.getShaderManager().getShader("post_outline"),
                 renderSizeLoc, &renderSize, SHADER_UNIFORM_VEC2);

  RenderTexture2D target =
      LoadRenderTexture(GetRenderWidth(), GetRenderHeight());

  while (!engine.ShouldClose()) {
    world.Update();
    if (useMouseInput)
      player.Input();
    player.Update();
    // BEFORE VISUAL CHANGE

    BeginTextureMode(target);
    ClearBackground(SKYBLUE);

    BeginMode3D(player.getCamera());
    BeginShaderMode(engine.getShaderManager().getShader("cel"));
    world.GetWorld().Draw();
    player.Draw();
    EndShaderMode();
    EndMode3D();
    EndTextureMode();
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Activate the edge-detection shader
    BeginShaderMode(engine.getShaderManager().getShader("post_outline"));
    // Draw the render texture matching full screen space.
    // NOTE: Render textures are inverted on the Y axis natively in OpenGL,
    // so we use a negative height source rectangle to flip it right-side up.
    DrawTexturePro(
        target.texture,
        (Rectangle){0, 0, (float)target.texture.width,
                    (float)-target.texture.height},
        (Rectangle){0, 0, (float)GetRenderWidth(), (float)GetRenderHeight()},
        (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();

    EndDrawing();
  }
  return 0;
}
