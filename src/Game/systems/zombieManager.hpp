#pragma once

#include <engine/entities.hpp>

#include <engine/world/quakeBsp.hpp>
#include <game/enemies/zombie.hpp>
#include <vector>

namespace game {

struct info_zombie_spawn {
  info_zombie_spawn(b3Vec3 _pos) : position(_pos) {};
  b3Vec3 position;
  int spawnrate = 1; // Spawn Zombie Rate
  int limit = -1;    // Max Spawn Ammount | Special Case
  int enabled = 1;   // Should Update
  int spawned = 0;   // Spawn Count Tracker
};

class ZombieManager {
public:
  void ReadLevelData(engine::QuakeBspLevel &quakeLevel);
  void UpdateSpawners();
  void UpdateZombies();

private:
  std::vector<info_zombie_spawn> zombieSpawners;
  std::vector<Zombie> ActiveZombies;
  std::vector<Zombie> inActiveZombies;
};

} // namespace game
