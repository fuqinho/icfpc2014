#ifndef SIMULATOR_GAMEOBJECT_H_
#define SIMULATOR_GAMEOBJECT_H_

#include <cassert>

#include "common.h"

class GameState;

enum class Direction {
  UP, RIGHT, DOWN, LEFT,
};

struct Position {
  int x;
  int y;
};

inline Position GetNextPosition(const Position& p, Direction d) {
  switch (d) {
    case Direction::UP:
      return { p.x, p.y - 1 };
    case Direction::RIGHT:
      return { p.x + 1, p.y };
    case Direction::DOWN:
      return { p.x, p.y + 1 };
    case Direction::LEFT:
      return { p.x - 1, p.y };
  }
  assert(false);
  return Position {};
}

class GameObject {
 public:
  GameObject() : direction_(Direction::DOWN) {
  }

  virtual ~GameObject() {
  }

  virtual Direction GetNextDirection(const GameState& ) = 0;

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
  Position position_;
  Direction direction_;
  int next_ticks_;

  DISALLOW_COPY_AND_ASSIGN(GameObject);
};

#endif
