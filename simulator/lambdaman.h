#ifndef SIMULATOR_LAMBDAMAN_H_
#define SIMULATOR_LAMBDAMAN_H_

#include "common.h"
#include "gameobject.h"

class LambdaMan : public GameObject {
 public:
  LambdaMan();
  virtual Direction GetNextDirection(const GameState& state);

  int life() const { return life_; }
  void set_life(int life) { life_ = life; }

  int vitality() const { return vitality_; }
  void set_vitality(int vitality) { vitality_ = vitality; }

 private:
  int life_;
  int vitality_;

  DISALLOW_COPY_AND_ASSIGN(LambdaMan);
};

#endif
