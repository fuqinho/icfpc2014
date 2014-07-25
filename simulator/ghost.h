#ifndef SIMULATOR_GHOST_H_
#define SIMULATOR_GHOST_H_

#include "common.h"
#include "gameobject.h"
#include "ghc_machine.h"
#include "lambdaman.h"

enum class GhostVitality {
  STANDARD, FRIGHT, INVISIBLE,
};

class Ghost : public GameObject, GHCMachineListener {
 public:
  Ghost(int index, int ai_id)
      : index_(index),
        ai_id_(ai_id),
        vitality_(GhostVitality::STANDARD),
        game_state_(NULL),
        last_direction_(Direction::RIGHT) {
    std::vector<std::string> lines;
    lines.push_back("mov a,255");
    lines.push_back("mov b,0");
    lines.push_back("mov c,255");
    lines.push_back("inc c");
    lines.push_back("jgt 7,[c],a");
    lines.push_back("mov a,[c]");
    lines.push_back("mov b,c");
    lines.push_back("jlt 3,c,3");
    lines.push_back("mov a,b");
    lines.push_back("int 0");
    lines.push_back("int 3");
    lines.push_back("int 6");
    lines.push_back("inc [b]");
    lines.push_back("hlt");
    machine_.LoadProgram(lines);
    machine_.SetListener(this);
  }

  virtual Direction GetNextDirection(const GameState& game_state) override {
    game_state_ = &game_state;
    machine_.Execute();
    return last_direction_;
  }

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

  virtual void OnInt(int i, std::vector<unsigned char>& registers) override {
    switch (i) {
      case 0:
        last_direction_ = static_cast<Direction>(registers[GHCRegister::A]);
        break;
      case 1:
        registers[GHCRegister::A] = game_state_->lambda_man().position().x;
        registers[GHCRegister::B] = game_state_->lambda_man().position().y;
        break;
      case 2:
        // TODO: second lamnba man
        break;
      case 3:
        registers[GHCRegister::A] = index_;
        break;
      case 4: {
        int index = registers[GHCRegister::A];
        registers[GHCRegister::A] = game_state_->ghost(index).initial_position().x;
        registers[GHCRegister::B] = game_state_->ghost(index).initial_position().y;
        break;
      }
      case 5: {
        int index = registers[GHCRegister::A];
        registers[GHCRegister::A] = game_state_->ghost(index).position().x;
        registers[GHCRegister::B] = game_state_->ghost(index).position().y;
        break;
      }
      case 6: {
        int index = registers[GHCRegister::A];
        registers[GHCRegister::A] = static_cast<unsigned char>(game_state_->ghost(index).vitality());
        registers[GHCRegister::B] = static_cast<unsigned char>(game_state_->ghost(index).direction());
        break;
      }
      case 7: {
        size_t x = registers[GHCRegister::A];
        size_t y = registers[GHCRegister::B];
        char tile = game_state_->tile({x, y});
        static const std::string tile_contents = "# .o%\\=";
        int pos = tile_contents.find(tile);
        registers[GHCRegister::A] = pos == std::string::npos ? 0 : pos;
        break;
      }
      case 8:
        std::cerr << "PC: " << static_cast<int>(registers[GHCRegister::PC]);
        for (size_t i = 0; i < 8; i++) {
          std::cerr << char('A' + i) << ": "
              << static_cast<int>(registers[i]) << ", ";
        }
        std::cerr << std::endl;
      default:
        break;
    }
  }

  DISALLOW_COPY_AND_ASSIGN(Ghost);
};

#endif
