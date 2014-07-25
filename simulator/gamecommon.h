#ifndef SIMULATOR_GAMECOMMON_H_
#define SIMULATOR_GAMECOMMON_H_

#include <cassert>

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
  size_t x;
  size_t y;
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

#endif
