#ifndef SIMULATOR_GAMEOBJECT_H_
#define SIMULATOR_GAMEOBJECT_H_

#include <cassert>

#include "common.h"

class GameState;

enum class Direction {
  UP, RIGHT, DOWN, LEFT,
};

inline Direction GetOppositeDirection(Direction d) {
  switch (d) {
    case Direction::UP: return Direction::DOWN;
    case Direction::RIGHT: return Direction::LEFT;
    case Direction::DOWN: return Direction::UP;
    case Direction::LEFT: return Direction::RIGHT;
  }
  assert(false);
  return Direction::DOWN;
}

struct Position {
  int x;
  int y;
};

bool operator==(const Position& p1, const Position& p2) {
  return p1.x == p2.x && p1.y == p2.y;
}

bool operator!=(const Position& p1, const Position& p2) {
  return !(p1 == p2);
}

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
