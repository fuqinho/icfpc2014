#ifndef SIMULATOR_TEST_AI_H_
#define SIMULATOR_TEST_AI_H_

#include <vector>
#include <queue>
#include <set>
#include <map>
#include "gamestate.h"

using namespace std;

template<class T1,class T2> ostream& operator<<(ostream& o,const pair<T1,T2>& p){return o<<"("<<p.first<<","<<p.second<<")";}
template<class T> ostream& operator<<(ostream& o, const set<T>& s){o<<"{";for(typename set<T>::const_iterator i=s.begin();i!=s.end();++i){if(i!=s.begin())o<<", ";o<<(*i);}o<<"}";return o;}
template<class K,class V> ostream& operator<<(ostream& o,const map<K,V>& m){o<<"{";for(typename map<K,V>::const_iterator i=m.begin();i!=m.end();++i){if(i!=m.begin())o<<", ";o<<i->first<<":"<<i->second;}o<<"}";return o;}

typedef std::pair<int,int> Coordinate; // (row, column)

//                       up  right down left
static const int DR[] = {-1,   0,   1,   0};
static const int DC[] = { 0,   1,   0,  -1};

bool IsInside(const Coordinate& coordinate, int height, int width) {
  return coordinate.first >= 0 && coordinate.first < height &&
         coordinate.second >= 0 && coordinate.second < width;
}

unsigned int TARGET_PILL = 1 << 0;
unsigned int TARGET_POWER = 1 << 1;
unsigned int TARGET_FRUIT = 1 << 2;
unsigned int TARGET_GHOST = 1 << 3;
unsigned int TARGET_SAFE = 1 << 4;

bool IsTarget(unsigned int targets, Coordinate coord, const std::set<Coordinate>& danger, const GameState& state) {
  if ((targets & TARGET_PILL) && state.game_map()[coord.first][coord.second] == '.') return true;
  if ((targets & TARGET_POWER) && state.game_map()[coord.first][coord.second] == 'o') return true;
  if ((targets & TARGET_FRUIT) && state.fruit() > 0 && state.game_map()[coord.first][coord.second] == '%') return true;
  if ((targets & TARGET_GHOST)) {
    for (int i = 0; i < state.ghost_size(); i++)
      if (coord.first == state.ghost(i).position().y && coord.second == state.ghost(i).position().x)
        return true;
  }
  if ((targets & TARGET_SAFE) && danger.count(coord) == 0) return true;
  return false;
}

pair<Direction, int> GetDirectionFor(Coordinate start, unsigned int targets, std::set<Coordinate>& danger, const GameState& state) {
  int H = (int)state.map_height();
  int W = (int)state.map_width();
  
  std::map<Coordinate, int> distance;
  std::map<Coordinate, int> direction;
  
  std::queue<Coordinate> que;
  que.push(start);
  distance[start] = 0;
  
  Coordinate goal(-1, -1);
  while (!que.empty() && goal == Coordinate(-1, -1)) {
    Coordinate current = que.front();
    que.pop();
    for (int i = 0; i < 4; i++) {
      Coordinate next(current.first + DR[i], current.second + DC[i]);
      if (!IsInside(next, H, W)) continue;
      if (distance.count(next)) continue;
      if (state.game_map()[next.first][next.second] == '#') continue;
      if (targets != TARGET_SAFE && danger.count(next)) continue;
      
      distance[next] = distance[current] + 1;
      direction[next] = i;
      que.push(next);
      
      if (IsTarget(targets, next, danger, state)) {
        goal = next;
        break;
      }
    }
  }
  cerr << distance << endl;
  //assert(goal != Coordinate(-1, -1));
  Coordinate to = goal;
  while (distance[to] > 1) {
    int dir = direction[to];
    to = Coordinate(to.first - DR[dir], to.second - DC[dir]);
  }
  return make_pair((Direction)direction[to], distance[goal]);
}

void FillVisitingCells(Coordinate cur, int dir, std::set<Coordinate>& visited, int turns, const GameState& state) {
  if (turns <= 0) return;
  if (visited.count(cur)) return;
  visited.insert(cur);
  for (int i = 0; i < 4; i++) {
    if (i == (dir ^ 2)) continue;
    Coordinate next(cur.first + DR[i], cur.second + DC[i]);
    if (IsInside(next, state.map_height(), state.map_width()) &&
        state.game_map()[next.first][next.second] != '#') {
      FillVisitingCells(next, i, visited, turns-1, state);
    }
  }
}

Direction GetNextDirectionGreedy(const GameState& state) {
  // Pre-calculate danger area.
  std::set<Coordinate> danger;
  if (state.lambda_man().vitality() < 500) {
    for (int i = 0; i < state.ghost_size(); i++) {
      std::set<Coordinate> visited;
      Coordinate coord(state.ghost(i).position().y, state.ghost(i).position().x);
      FillVisitingCells(coord, (int)state.ghost(i).direction(), visited, 3, state);
      for (auto it = visited.begin(); it != visited.end(); it++)
        danger.insert(*it);
    }
  }
  Coordinate start(state.lambda_man().position().y, state.lambda_man().position().x);
  std::cerr << "Now: " << start << std::endl;
  std::cerr << "Danger: " << danger << std::endl;
  for (int r = 0; r < state.map_height(); r++) {
    for (int c = 0; c < state.map_width(); c++) {
      if (state.game_map()[r][c] == '#')
        std::cerr << '#';
      else if (danger.count(Coordinate(r, c)))
        std::cerr << 'O';
      else
        std::cerr << ' ';
    }
    std::cerr << std::endl;
  }
  
  // If lambdaman is in Danger, go to safe area.
  if (danger.count(start)) {
    std::cerr << "Nigeyou!" << std::endl;
    pair<Direction, int> res = GetDirectionFor(start, TARGET_SAFE, danger, state);
    std::cerr << "Direction: " << (int)res.first;
    return res.first;
  }
  
  // If lambdaman has power and there're ghosts near him, go kill them.
  if (state.lambda_man().vitality() > 500) {
    std::cerr << "Go to Ghost!" << std::endl;
    pair<Direction, int> res = GetDirectionFor(start, TARGET_GHOST, danger, state);
    if (res.second <= 6)
      return res.first;
  }
  
  // If there is a fruit, get it.
  if (state.fruit() > 0) {
    std::cerr << "Go to Fruit!" << std::endl;
    return GetDirectionFor(start, TARGET_FRUIT, danger, state).first;
  }
  
  // By default, go to pills.
  std::cerr << "Go to Pills" << std::endl;
  return GetDirectionFor(start, TARGET_PILL|TARGET_POWER, danger, state).first;
}

#endif