#ifndef SIMULATOR_GHOST_H_
#define SIMULATOR_GHOST_H_

#include "common.h"
#include "gameobject.h"

class Ghost : public GameObject {
 public:
  Ghost();
  virtual Direction GetNextMove();

 private:
  DISALLOW_COPY_AND_ASSIGN(Ghost);
};

#endif
