#pragma once

#include "box3d/id.h"
#include "engine/engine.hpp"
#include "engine/world/entity.hpp"
#include "engine/world/entityRegistry.hpp"
#include <engine/physics/physics.hpp>
#include <engine/world/quakeBsp.hpp>
namespace engine {
class World {
public:
  explicit World(Engine &engine);
  ~World();

  void Init();
  void Shutdown();
  void Update();
  void Draw();
  void DrawGameObjects();
  void SpawnEntities();
  b3WorldId GetWorldId() const { return worldId; }
  std::vector<EntityData> &GetSceneObjects() { return sceneObjects; }
  bool LoadBspLevel(const std::filesystem::path &path);
  bool HasBspLevel() const { return quakeLevel.IsLoaded(); }
  const std::string &GetBspStatus() const { return bspStatus; }
  SpawnData GetBspPlayerSpawn() const { return quakeLevel.GetPlayerSpawn(); }
  bool HasBspPlayerSpawn() const { return quakeLevel.HasPlayerSpawn(); }
  std::vector<std::unique_ptr<RuntimeEntity>> &getRuntimeEntities() {
    return runtimeEntities;
  }
  void setRuntimeEntities(int index, std::unique_ptr<RuntimeEntity> val) {
    runtimeEntities[index].swap(val);
  }

private:
  void RegisterEntitySpawners();
  void ClearSpawnedEntities();
  void SpawnTrigger(const Entity &entity);
  void SpawnDynamicProp(const Entity &entity);
  void SpawnWallbuy(const Entity &entity);
  void SpawnInteractable(const Entity &entity);
  Engine &engine; // why is this here it should not by im not touching it
  EventListener eventListener;
  EntityRegistry entitySpawners;

  Model skyBox;

  b3WorldId worldId{};
  std::vector<EntityData> sceneObjects;
  QuakeBspLevel quakeLevel;
  std::vector<std::unique_ptr<RuntimeEntity>> runtimeEntities;

  std::string bspStatus = "Drop a Quake BSP29 file to load a level";
  bool isShutdown = false;

  float timeStep = 1.0f / 60.0f;
  int subStepCount = 8;
};

} // namespace engine
