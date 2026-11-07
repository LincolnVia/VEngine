#pragma once

/*
 *
 *
 */

namespace engine {

class Player;

class AIController {
public:
  explicit AIController(Player &player) : player(player) {}
  Player FindPlayer();
  void SetPlayer(Player &player);
  void WalkTo(Player &player);

private:
  Player &player;
};

} // namespace engine
