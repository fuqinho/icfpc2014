#ifndef SIMULATOR_GAMESTATE_H_
#define SIMULATOR_GAMESTATE_H_

#include <memory>
#include <vector>

#include "common.h"

class Ghost;
class LambdaMan;
typedef std::vector<std::string> GameMap;

class GameState {
 public:
  GameState() {
  }

  size_t map_width() const { return game_map_[0].size(); }
  size_t map_height() const { return game_map_.size(); }

  const GameMap& game_map() const { return game_map_; }
  GameMap* mutable_game_map() { return &game_map_; }

  const LambdaMan& lambda_man() const { return *lambda_man_; }
  LambdaMan* mutable_lambda_man() { return lambda_man_.get(); }

  const std::vector<std::unique_ptr<Ghost> >& ghost_list() const {
    return ghost_list_;
  }
  size_t ghost_size() const { return ghost_list_.size(); }

  const Ghost& ghost(size_t index) const { return *ghost_list_[index]; }
  Ghost* mutable_ghost(size_t index) { return ghost_list_[index].get(); }

  int fruit() const { return fruit_; }
  void set_fruit(int fruit) { fruit_ = fruit; }

 private:
  GameMap game_map_;
  std::unique_ptr<LambdaMan> lambda_man_;
  std::vector<std::unique_ptr<Ghost> > ghost_list_;

  int fruit_;

  DISALLOW_COPY_AND_ASSIGN(GameState);
};

#endif
