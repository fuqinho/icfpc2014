#ifndef SIMULATOR_GAMEOBJECT_H_
#define SIMULATOR_GAMEOBJECT_H_

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
  GameObject() : current_direction_(Direction::DOWN) {
  }

  virtual ~GameObject() {
  }

  virtual Direction GetNextMove(const GameState& ) = 0;

  const Position& position() const { return position_; }
  void set_position(const Position& p) {
    position_ = p;
  }

  const Direction current_direction() const { return current_direction_; }
  void set_current_direction(const Direction& d) {
    current_direction_ = d;
  }

 private:
  Position position_;
  Direction current_direction_;

  DISALLOW_COPY_AND_ASSIGN(GameObject);
};

#endif
