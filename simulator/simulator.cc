#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "common.h"
#include "gamestate.h"
#include "ghost.h"
#include "lambdaman.h"

#include "glog/logging.h"

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

  void set_map_file(const std::string& map_file) {
    map_file_ = map_file;
  }

  void add_ai(const std::string& ai_file) {
    ai_file_list_.push_back(ai_file);
  }

  void Run() {
    Init();
    int end_of_ticks =
        127 * game_state_.map_width() * game_state_.map_height() * 16;
    // Clear screen and disable cursor.
    printf("\x1b[2J\x1b[?25l");
    fflush(stdout);
    PrintGame(0);
    for (int ticks = 1; ticks < end_of_ticks; ++ticks) {
      if (RunStep(ticks))
        break;
      PrintGame(ticks);
    }
    PrintGame(end_of_ticks);
  }

 private:
  void Init() {
    LoadMap(game_state_.mutable_game_map(), map_file_);
    game_state_.set_lambda_man(std::unique_ptr<LambdaMan>(new LambdaMan));
    InitLambdaMan(game_state_.mutable_lambda_man(), game_state_.game_map());
    InitGhostList(game_state_.mutable_ghost_list(), game_state_.game_map());
    game_state_.set_fruit(0);
    game_state_.set_score(0);
  }

  static void LoadMap(GameMap* game_map, const std::string& map_file) {
    std::ifstream in(map_file);
    std::string line;
    while (std::getline(in, line)) {
      game_map->push_back(line);
    }
  }

  static void InitLambdaMan(LambdaMan* lambda_man, const GameMap& game_map) {
    for (size_t y = 0; y < game_map.size(); ++y) {
      for (size_t x = 0; x < game_map[y].size(); ++x) {
        if (game_map[y][x] == '\\') {
          lambda_man->set_initial_position(Position {x, y});
          lambda_man->set_position(Position {x, y});
          lambda_man->set_direction(Direction::DOWN);
          lambda_man->set_life(3);
          lambda_man->set_next_ticks(127);
          return;
        }
      }
    }
  }

  static void InitGhostList(std::vector<std::unique_ptr<Ghost> >* ghost_list,
                            const GameMap& game_map) {
    int ai_id = 1;  // TODO
    for (size_t y = 0; y < game_map.size(); ++y) {
      for (size_t x = 0; x < game_map[y].size(); ++x) {
        if (game_map[y][x] == '=') {
          std::unique_ptr<Ghost> ptr(new Ghost(ai_id));
          ptr->set_initial_position(Position {x, y});
          ptr->set_position(Position {x, y});
          ptr->set_direction(Direction::DOWN);
          ptr->set_next_ticks(kGhostTicks[ai_id][0]);
          ghost_list->push_back(std::move(ptr));
        }
      }
    }
  }

  bool RunStep(int current_ticks) {
    MovePhase(current_ticks);
    ActionPhase(current_ticks);
    EatPillPhase(current_ticks);
    EatGhostPhase();
    return GameOverCheckPhase();
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

    Direction next = obj->GetNextDirection(game_state_);
    Position position = obj->position();
    size_t num_walls =
        (game_state_.tile(GetNextPosition(position, Direction::UP)) == '#') +
        (game_state_.tile(GetNextPosition(position, Direction::RIGHT)) == '#') +
        (game_state_.tile(GetNextPosition(position, Direction::DOWN)) == '#') +
        (game_state_.tile(GetNextPosition(position, Direction::LEFT)) == '#');
    if (num_walls == 3) {
      // Dead end.
      MoveIfAvailable(obj, Direction::UP, game_state_.game_map()) ||
      MoveIfAvailable(obj, Direction::RIGHT, game_state_.game_map()) ||
      MoveIfAvailable(obj, Direction::DOWN, game_state_.game_map()) ||
      MoveIfAvailable(obj, Direction::LEFT, game_state_.game_map());
    } else if (num_walls == 2) {
      // Corrido or bend.
      (obj->direction() != Direction::DOWN &&
       MoveIfAvailable(obj, Direction::UP, game_state_.game_map())) ||
      (obj->direction() != Direction::LEFT &&
       MoveIfAvailable(obj, Direction::RIGHT, game_state_.game_map())) ||
      (obj->direction() != Direction::UP &&
       MoveIfAvailable(obj, Direction::DOWN, game_state_.game_map())) ||
      (obj->direction() != Direction::RIGHT &&
       MoveIfAvailable(obj, Direction::LEFT, game_state_.game_map()));
    } else {
      // Junction.
      (next != GetOppositeDirection(obj->direction()) &&
       MoveIfAvailable(obj, next, game_state_.game_map())) ||
      MoveIfAvailable(obj, obj->direction(), game_state_.game_map()) ||
      MoveIfAvailable(obj, Direction::UP, game_state_.game_map()) ||
      MoveIfAvailable(obj, Direction::RIGHT, game_state_.game_map()) ||
      MoveIfAvailable(obj, Direction::DOWN, game_state_.game_map()) ||
      MoveIfAvailable(obj, Direction::LEFT, game_state_.game_map());
    }
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

    // This may be overwritten in following checks.
    obj->set_next_ticks(current_ticks + 127);
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

  void EatPillPhase(int current_ticks) {
    Position position = game_state_.lambda_man().position();
    char tile = game_state_.tile(position);
    if (tile == '.') {
      // Eat pill.
      game_state_.add_score(10);
      game_state_.mutable_lambda_man()->set_next_ticks(137 + current_ticks);
      *game_state_.mutable_tile(position) = ' ';
    } else if (tile == 'o') {
      // Eat power pill.
      game_state_.add_score(50);
      game_state_.mutable_lambda_man()->set_next_ticks(137 + current_ticks);
      *game_state_.mutable_tile(position) = ' ';
      size_t num_ghost = game_state_.ghost_size();
      for (size_t i = 0; i < num_ghost; ++i) {
        Ghost* ghost = game_state_.mutable_ghost(i);
        if (ghost->vitality() == GhostVitality::STANDARD) {
          ghost->set_vitality(GhostVitality::FRIGHT);
        }
        ghost->set_direction(GetOppositeDirection(ghost->direction()));
      }
    } else if (tile == '%' && game_state_.fruit() > 0) {
      // Eat fruit.
      game_state_.add_score(game_state_.fruit_score());
      game_state_.set_fruit(0);
      // Hack.
      if (game_state_.lambda_man().next_ticks() ==
          current_ticks + 127) {
        game_state_.mutable_lambda_man()->set_next_ticks(current_ticks + 137);
      }
    }
  }

  void EatGhostPhase() {
    Position position = game_state_.lambda_man().position();
    size_t num_ghost = game_state_.ghost_size();
    for (size_t i = 0; i < num_ghost; ++i) {
      Ghost* ghost = game_state_.mutable_ghost(i);
      if (ghost->vitality() == GhostVitality::INVISIBLE) {
        continue;
      }
      if (ghost->position() != position) {
        continue;
      }

      if (ghost->vitality() == GhostVitality::FRIGHT) {
        // TODO score.
        ghost->set_position(ghost->initial_position());
        ghost->set_vitality(GhostVitality::INVISIBLE);
        ghost->set_direction(Direction::DOWN);
      } else {
        // Loose a life.
        game_state_.mutable_lambda_man()->set_life(
            game_state_.lambda_man().life() - 1);
        game_state_.mutable_lambda_man()->set_position(
            game_state_.lambda_man().initial_position());
        game_state_.mutable_lambda_man()->set_direction(Direction::DOWN);
        for (size_t j = 0; j < num_ghost; ++j) {
          Ghost* ghost2 = game_state_.mutable_ghost(j);
          ghost2->set_position(ghost2->initial_position());
          ghost2->set_direction(Direction::DOWN);
        }
        break;
      }
    }
  }

  bool GameOverCheckPhase() {
    // Check if lambdaman wins.
    bool won = true;
    for (size_t y = 0; y < game_state_.map_height(); ++y) {
      for (size_t x = 0; x < game_state_.map_width(); ++x) {
        if (game_state_.game_map()[y][x] == '.') {
          won = false;
          break;
        }
      }
    }
    if (won) {
      game_state_.set_score(
          game_state_.score() * (game_state_.lambda_man().life() + 1));
      return true;
    }

    // Check if game is over.
    if (game_state_.lambda_man().life() == 0) {
      return true;
    }

    return false;
  }

  std::string map_file_;
  // TODO lambda ai file.
  std::vector<std::string> ai_file_list_;

  GameState game_state_;

  void PrintGame(int current_ticks) const {
    std::stringstream buffer;
    buffer << "\x1b[H";
    buffer << "ticks: " << current_ticks
           << ", score: " << game_state_.score() << "\n";
    for (size_t y = 0; y < game_state_.map_height(); ++y) {
      for (size_t x = 0; x < game_state_.map_width(); ++x) {
        if (game_state_.lambda_man().position() == Position {x, y}) {
          buffer << "\\";
          continue;
        }
        size_t num_ghost = game_state_.ghost_size();
        bool is_ghost = false;
        for (size_t i = 0; i < num_ghost; ++i) {
          if (game_state_.ghost(i).position() == Position {x, y}) {
            is_ghost = true;
            break;
          }
        }
        if (is_ghost) {
          buffer << "=";
          continue;
        }
        if (game_state_.game_map()[y][x] == '=' ||
            game_state_.game_map()[y][x] == '\\') {
          buffer << " ";
          continue;
        }

        if (game_state_.game_map()[y][x] == '%') {
          if (game_state_.fruit() > 0) {
            buffer << "%";
          } else {
            buffer << " ";
          }
          continue;
        }

        buffer << game_state_.game_map()[y][x];
      }
      buffer << "\n";
    }
    const std::string str = buffer.str();
    write(1, str.c_str(), str.size());
  }
  // TODO event optimization.

  DISALLOW_COPY_AND_ASSIGN(Simulator);
};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  Simulator sim;
  sim.set_map_file(argv[1]);
  sim.Run();
  return 0;
}
