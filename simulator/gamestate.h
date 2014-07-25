#ifndef SIMULATOR_GAMESTATE_H_
#define SIMULATOR_GAMESTATE_H_

#include "common.h"

class Ghost;
class LambdaMan;
typedef std::vector<std::string> GameMap;

class GameState {
 public:
  GameState() {
  }

  const GameMap& game_map() const { return game_map_; }
  const LambdaMan& lambda_man() const { return *lambda_man_; }
  const std::vector<std::unique_ptr<Ghost> >& ghost_list() const {
    return *ghost_list_;
  }

 private:
  GameMap game_map_;
  std::unique_ptr<LambdaMan> lambda_man_;
  std::vector<std::unique_ptr<Ghost> > ghost_list_;

  DISALLOW_COPY_AND_ASSIGN(GameState);
};

#endif
