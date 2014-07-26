#ifndef SIMULATOR_LAMBDAMAN_H_
#define SIMULATOR_LAMBDAMAN_H_

#include <string>

#include "common.h"
#include "gamecommon.h"
#include "gameobject.h"
#include "gamestate.h"
#include "gcc.h"

class GameState;

class LambdaMan : public GameObject {
 public:
  LambdaMan();
  ~LambdaMan();

  void LoadProgram(const std::string& program);
  void Init(const GameState& state);

  virtual Direction GetNextDirection(const GameState& state) override;

  int life() const { return life_; }
  void set_life(int life) { life_ = life; }

  int vitality() const { return vitality_; }
  void set_vitality(int vitality) { vitality_ = vitality; }

 private:
  Value BuildWorld(const GameState& game_state);
  Value BuildMap(const GameMap& m);
  Value BuildLambdaMan(const LambdaMan& lambdaman, int score);
  Value BuildGhostList(
      const std::vector<std::unique_ptr<Ghost> >& ghost_list);
  Value BuildGhost(const Ghost& ghost);
  Value BuildFruit(int fruit);

  int life_;
  int vitality_;

  GCC gcc_;
  Value ai_state_;
  Value step_function_;

  DISALLOW_COPY_AND_ASSIGN(LambdaMan);
};

#endif
