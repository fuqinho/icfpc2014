#ifndef SIMULATOR_LAMBDAMAN_H_
#define SIMULATOR_LAMBDAMAN_H_

#include "common.h"
#include "gameobject.h"

class LambdaMan : public GameObject {
 public:
  LambdaMan();
  virtual Direction GetNextDirection(const GameState& state);

 private:
  DISALLOW_COPY_AND_ASSIGN(LambdaMan);
};

#endif
