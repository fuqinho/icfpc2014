#ifndef SIMULATOR_GAMESTATE_H_
#define SIMULATOR_GAMESTATE_H_

#include <memory>
#include <vector>

#include "common.h"
#include "gamecommon.h"

class Ghost;
class LambdaMan;
typedef std::vector<std::string> GameMap;

class GameState {
 public:
  GameState() : game_level_(1) {
  }

  size_t map_width() const { return game_map_[0].size(); }
  size_t map_height() const { return game_map_.size(); }
  int game_level() const { return game_level_; }
  void set_game_level(int level) { game_level_ = level; }

  const GameMap& game_map() const { return game_map_; }
  GameMap* mutable_game_map() { return &game_map_; }
  char tile(const Position& p) const { return game_map_[p.y][p.x]; }
  char* mutable_tile(const Position& p) { return &game_map_[p.y][p.x]; }

  const LambdaMan& lambda_man() const { return *lambda_man_; }
  void set_lambda_man(std::unique_ptr<LambdaMan> lambda_man) {
    lambda_man_ = std::move(lambda_man);
  }
  LambdaMan* mutable_lambda_man() { return lambda_man_.get(); }

  const std::vector<std::unique_ptr<Ghost> >& ghost_list() const {
    return ghost_list_;
  }
  std::vector<std::unique_ptr<Ghost> >* mutable_ghost_list() {
    return &ghost_list_;
  }
  size_t ghost_size() const { return ghost_list_.size(); }

  const Ghost& ghost(size_t index) const { return *ghost_list_[index]; }
  Ghost* mutable_ghost(size_t index) { return ghost_list_[index].get(); }

  int fruit() const { return fruit_; }
  void set_fruit(int fruit) { fruit_ = fruit; }

  int fruit_score() const {
    switch (game_level()) {
      case 1: return 100;
      case 2: return 300;
      case 3: return 500;
      case 4: return 500;
      case 5: return 700;
      case 6: return 700;
      case 7: return 1000;
      case 8: return 1000;
      case 9: return 2000;
      case 10: return 2000;
      case 11: return 3000;
      case 12: return 3000;
      default: return 5000;
    }
  }

  int score() const { return score_; }
  void add_score(int score) { score_ += score; }
  void set_score(int score) { score_ = score; }

 private:
  GameMap game_map_;
  std::unique_ptr<LambdaMan> lambda_man_;
  std::vector<std::unique_ptr<Ghost> > ghost_list_;

  int fruit_;
  int score_;
  int game_level_;

  DISALLOW_COPY_AND_ASSIGN(GameState);
};

#endif
