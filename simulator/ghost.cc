#include "common.h"
#include "ghost.h"
#include "lambdaman.h"
#include "ghc_instruction.h"

Ghost::Ghost(int index, int ai_id, const std::string& program)
    : index_(index),
      ai_id_(ai_id),
      vitality_(GhostVitality::STANDARD),
      game_state_(NULL),
      last_direction_(Direction::DOWN) {
  machine_.LoadProgram(program);
  machine_.SetListener(this);
}

Ghost::~Ghost() {
}

Direction Ghost::GetNextDirection(const GameState& game_state) {
  game_state_ = &game_state;
  machine_.Execute();
  return last_direction_;
}

void Ghost::OnInt(int i, std::vector<unsigned char>& registers) {
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
      if (index >= 0 && index < game_state_->ghost_size()) {
        registers[GHCRegister::A] =
            game_state_->ghost(index).initial_position().x;
        registers[GHCRegister::B] =
            game_state_->ghost(index).initial_position().y;
      }
      break;
    }
    case 5: {
      int index = registers[GHCRegister::A];
      if (index >= 0 && index < game_state_->ghost_size()) {
        registers[GHCRegister::A] = game_state_->ghost(index).position().x;
        registers[GHCRegister::B] = game_state_->ghost(index).position().y;
      }
      break;
    }
    case 6: {
      int index = registers[GHCRegister::A];
      if (index >= 0 && index < game_state_->ghost_size()) {
        registers[GHCRegister::A] =
            static_cast<unsigned char>(game_state_->ghost(index).vitality());
        registers[GHCRegister::B] =
            static_cast<unsigned char>(game_state_->ghost(index).direction());
      }
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
