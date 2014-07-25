#include "common.h"

#include "gamestate.h"
#include "ghost.h"
#include "lambdaman.h"

#include <vector>
#include <string>

const int kGhostTicks[4][3] = {
  { 130, 195, 130, },
  { 132, 198, 132, },
  { 134, 201, 134, },
  { 136, 204, 136, },
};

class Simulator {
 public:
  Simulator() {
  }

  void Run() {
    Init();
    int end_of_ticks =
        127 * game_state_.map_width() * game_state_.map_height() * 16;
    for (int ticks = 1; ticks < end_of_ticks; ++ticks) {
      RunStep(ticks);
    }
  }

 private:
  void Init() {
    // TODO setup next ticks.
  }

  void RunStep(int current_ticks) {
    MovePhase(current_ticks);
    ActionPhase(current_ticks);
    EatPillPhase();
    EatGhostPhase();
    GameOverCheckPhase();
  }

  void MovePhase(int current_ticks) {
    size_t num_ghost = game_state_.ghost_size();
    for (size_t i = 0; i < num_ghost; ++i) {
      MoveGhost(game_state_.mutable_ghost(i), current_ticks);
    }

    MoveLambdaMan(game_state_.mutable_lambda_man(), current_ticks);
  }

  void MoveGhost(Ghost* obj, int current_ticks) {
    if (obj->next_ticks() > current_ticks) {
      return;
    }

    MoveIfAvailable(obj, obj->GetNextDirection(game_state_),
                    game_state_.game_map()) ||
        MoveIfAvailable(obj, obj->direction(), game_state_.game_map()) ||
        MoveIfAvailable(obj, Direction::UP, game_state_.game_map()) ||
        MoveIfAvailable(obj, Direction::RIGHT, game_state_.game_map()) ||
        MoveIfAvailable(obj, Direction::DOWN, game_state_.game_map()) ||
        MoveIfAvailable(obj, Direction::LEFT, game_state_.game_map());
    obj->set_next_ticks(
        current_ticks +
        kGhostTicks[obj->ai_id()][static_cast<int>(obj->vitality())]);
  }

  void MoveLambdaMan(LambdaMan* obj, int current_ticks) {
    if (obj->next_ticks() > current_ticks) {
      return;
    }

    Direction d = obj->GetNextDirection(game_state_);
    MoveIfAvailable(obj, d, game_state_.game_map());
    obj->set_direction(d);

    obj->set_next_ticks(
        current_ticks + (obj->vitality() > 0 ? 137 : 127));
  }

  static bool MoveIfAvailable(
      GameObject* obj, Direction d, const GameMap& game_map) {
    Position next = GetNextPosition(obj->position(), d);
    if (game_map[next.y][next.x] == '#') {
      return false;
    }

    obj->set_position(next);
    obj->set_direction(d);
    return true;
  }

  void ActionPhase(int current_ticks) {
    // Cancel fright mode.
    {
      int vitality = game_state_.lambda_man().vitality();
      if (vitality > 0) {
        --vitality;
        game_state_.mutable_lambda_man()->set_vitality(vitality);
        if (vitality == 0) {
          size_t num_ghost = game_state_.ghost_size();
          for (size_t i = 0; i < num_ghost; ++i) {
            game_state_.mutable_ghost(i)->set_vitality(
                GhostVitality::STANDARD);
          }
        }
      }
    }

    // Handling fruit.
    if (current_ticks == 127 * 200 || current_ticks == 127 * 400) {
      // Fruit will disappear after 127 * 80 ticks.
      game_state_.set_fruit(127 * 80);
    } else {
      int fruit = game_state_.fruit();
      if (fruit > 0)
        game_state_.set_fruit(fruit - 1);
    }
  }

  void EatPillPhase() {
  }

  void EatGhostPhase() {
  }

  void GameOverCheckPhase() {
  }

  GameState game_state_;

  DISALLOW_COPY_AND_ASSIGN(Simulator);
};

int main() {
  Simulator sim;
  sim.Run();
  return 0;
}
