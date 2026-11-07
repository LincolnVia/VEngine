#pragma once

#include <engine/world/quakeBsp.hpp>
#include <game/enemies/zombie.hpp>
#include <game/world/entities.hpp>
#include <vector>

namespace game {

struct info_hound_spawn {
  info_hound_spawn(b3Vec3 _pos) : position(_pos) {};
  b3Vec3 position;
  int spawnrate = 1; // Spawn Zombie Rate
  int limit = -1;    // Max Spawn Ammount | Special Case
  int enabled = 1;   // Should Update
  int spawned = 0;   // Spawn Count Tracker
};

class HoundManager {
public:
  void ReadLevelData(QuakeBspLevel &quakeLevel);
  void UpdateSpawners();
  void UpdateZombies();

private:
  std::vector<info_hound_spawn> houndSpawners;
  std::vector<Zombie> ActiveHounds;
  std::vector<Zombie> inActiveHounds;
};

} // namespace game
