#ifndef SIMULATOR_GAMEOBJECT_H_
#define SIMULATOR_GAMEOBJECT_H_

#include "common.h"
#include "gamecommon.h"

class GameState;

class GameObject {
 public:
  GameObject() : direction_(Direction::DOWN) {
  }

  virtual ~GameObject() {
  }

  virtual Direction GetNextDirection(const GameState& ) = 0;

  const Position& initial_position() const { return initial_position_; }
  void set_initial_position(const Position& p) {
    initial_position_ = p;
  }

  const Position& position() const { return position_; }
  void set_position(const Position& p) {
    position_ = p;
  }

  const Direction direction() const { return direction_; }
  void set_direction(const Direction& d) {
    direction_ = d;
  }

  int next_ticks() const { return next_ticks_; }
  void set_next_ticks(int next_ticks) { next_ticks_ = next_ticks; }

 private:
  Position initial_position_;
  Position position_;
  Direction direction_;
  int next_ticks_;

  DISALLOW_COPY_AND_ASSIGN(GameObject);
};

#endif
