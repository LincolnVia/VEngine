#ifndef ENGINE_H
#define ENGINE_H

#include "assetManager.hpp"
#include "eventListener.hpp"
#include "shaderManager.hpp"

#include <cstdio>
#include <filesystem>
#include <functional>
#include <iostream>
#include <ostream>
#include <raylib.h>
#include <raymath.h>

namespace engine {

class Engine {
public:
  Vector2 winSize = {1280, 720};
  const char *winTitle = "Box3D";

  explicit Engine(const char *title = "Box3D");
  AssetManager &getAssetManager() { return assetManager; }
  ShaderManager &getShaderManager() { return shaderManager; }
  bool ShouldClose() const;
  void Process(const std::function<void()> &frame = nullptr);
  void ProcessFrame(const std::function<void()> &drawOverlay = nullptr);
  ~Engine();

private:
  AssetManager assetManager;
  ShaderManager shaderManager;
};

} // namespace engine
#endif
