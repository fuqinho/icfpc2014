#ifndef SIMULATOR_GHOST_H_
#define SIMULATOR_GHOST_H_

#include "common.h"
#include "gameobject.h"

enum class GhostVitality {
  STANDARD, FRIGHT, INVISIBLE,
};

class Ghost : public GameObject {
 public:
  Ghost(int ai_id)
      : ai_id_(ai_id),
        vitality_(GhostVitality::STANDARD) {
  }
  virtual Direction GetNextDirection(const GameState& game_state) override;

  int ai_id() const { return ai_id_; }
  GhostVitality vitality() const { return vitality_; }
  void set_vitality(GhostVitality vitality) { vitality_ = vitality; }

 private:
  int ai_id_;
  GhostVitality vitality_;

  DISALLOW_COPY_AND_ASSIGN(Ghost);
};

#endif
