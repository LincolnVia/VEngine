#include "engine.hpp"
#include <raylib.h>

namespace engine {
Engine::Engine(const char *title) : winTitle(title) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(static_cast<int>(winSize.x), static_cast<int>(winSize.y),
             winTitle);
  SetTargetFPS(60);
}

Engine::~Engine() { CloseWindow(); }

bool Engine::ShouldClose() const { return WindowShouldClose(); }

void Engine::Process(const std::function<void()> &frame) {
  while (!ShouldClose())
    ProcessFrame(frame);
}

void Engine::ProcessFrame(const std::function<void()> &drawOverlay) {
  BeginDrawing();
  ClearBackground(BLACK);
  if (drawOverlay)
    drawOverlay();

  EndDrawing();
}
} // namespace engine
