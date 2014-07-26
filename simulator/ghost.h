#ifndef SIMULATOR_GHOST_H_
#define SIMULATOR_GHOST_H_

#include "common.h"
#include "gameobject.h"
#include "ghc_machine.h"

enum class GhostVitality {
  STANDARD, FRIGHT, INVISIBLE,
};

class Ghost : public GameObject, GHCMachineListener {
 public:
  Ghost(int index, int ai_id, const std::string& program);
  ~Ghost();

  virtual Direction GetNextDirection(const GameState& game_state) override;

  int index() const { return index_; }
  int ai_id() const { return ai_id_; }
  GhostVitality vitality() const { return vitality_; }
  void set_vitality(GhostVitality vitality) { vitality_ = vitality; }

 private:
  int index_;
  int ai_id_;
  GhostVitality vitality_;
  GHCMachine machine_;
  const GameState* game_state_;
  Direction last_direction_;

  virtual void OnInt(int i, std::vector<unsigned char>& registers) override;

  DISALLOW_COPY_AND_ASSIGN(Ghost);
};

#endif
