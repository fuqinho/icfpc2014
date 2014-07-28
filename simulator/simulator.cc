#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <signal.h>

#include "common.h"
#include "gamestate.h"
#include "ghost.h"
#include "lambdaman.h"
#include "mapstore.h"

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

  void set_lambdaman_file(const std::string& lambdaman_file) {
    lambdaman_file_ = lambdaman_file;
  }

  void add_ai_file(const std::string& ai_file) {
    ai_file_list_.push_back(ai_file);
  }

  int Run(bool visualize) {
    Init();
    int end_of_ticks =
        127 * game_state_.map_width() * game_state_.map_height() * 16;
    // Clear screen and disable cursor.
    if (visualize) {
      printf("\x1b[2J\x1b[?25l");
      fflush(stdout);
      PrintGame(0);
    }
    for (int ticks = 1; ticks < end_of_ticks; ++ticks) {
      someone_moved_ = false;
      if (RunStep(ticks))
        break;
      if (visualize && someone_moved_) {
        PrintGame(ticks);
        std::cin.ignore();
      }
    }
    if (visualize)
      PrintGame(end_of_ticks);
    return game_state_.score();
  }

 private:
  void Init() {
    game_state_.set_lambda_man(std::unique_ptr<LambdaMan>(new LambdaMan));
    LoadLambdaMan(game_state_.mutable_lambda_man(), lambdaman_file_);
    InitLevel(1);
  }

  void InitLevel(int level) {
    game_state_.set_game_level(level);
    LoadMap(game_state_.mutable_game_map(), map_file_);
    InitLambdaMan(game_state_.mutable_lambda_man(),  game_state_.game_map());
    InitGhostList(game_state_.mutable_ghost_list(),
                  ai_file_list_,
                  game_state_.game_map());
    game_state_.set_fruit(0);
    game_state_.set_score(0);
    game_state_.mutable_lambda_man()->Init(game_state_);
  }

  static void LoadLambdaMan(LambdaMan* lambda_man,
                            const std::string& lambdaman_file) {
    std::ifstream ifs(lambdaman_file);
    ifs.seekg(0, std::ios::end);
    int len = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::string data;
    data.resize(len);
    ifs.read(&data[0], len);
    lambda_man->LoadProgram(data);
  }

  static void InitLambdaMan(LambdaMan* lambda_man,
                            const GameMap& game_map) {
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
                            const std::vector<std::string>& ai_file_list,
                            const GameMap& game_map) {
    std::vector<std::string> ai_list;
    for (size_t i = 0; i < ai_file_list.size(); ++i) {
      std::ifstream ifs(ai_file_list[i]);
      ifs.seekg(0, std::ios::end);
      int len = ifs.tellg();
      ifs.seekg(0, std::ios::beg);
      std::string data;
      data.resize(len);
      ifs.read(&data[0], len);
      ai_list.push_back(std::move(data));
    }

    ghost_list->clear();
    int num_ghosts = 0;
    for (size_t y = 0; y < game_map.size(); ++y) {
      for (size_t x = 0; x < game_map[y].size(); ++x) {
        if (game_map[y][x] == '=') {
          int ai = num_ghosts % ai_list.size();
          std::unique_ptr<Ghost> ptr(
              new Ghost(num_ghosts++, ai, ai_list[ai]));
          ptr->set_initial_position(Position {x, y});
          ptr->set_position(Position {x, y});
          ptr->set_direction(Direction::DOWN);
          ptr->set_next_ticks(kGhostTicks[ptr->index() % 4][0]);
          ghost_list->push_back(std::move(ptr));
        }
      }
    }
  }

  void LoadMap(GameMap* game_map, const std::string& map_file) {
    // Get map by ID.
    std::vector<std::string> map_content = GetMap(map_file);
    
    // If no map gotten, try to load file.
    if (map_content.empty()) {
      std::ifstream in(map_file);
      std::string line;
      while (std::getline(in, line))
        map_content.push_back(line);
    }
    if (map_content.empty()) {
      std::cerr << "Not found: " << map_file << std::endl;
      exit(1);
    }
    for (auto line : map_content)
      game_map->push_back(line);

    // Calculate game level.
    int area = (int)(*game_map).size() * (int)(*game_map)[0].size();
    int level = 1;
    while (100 * level < area) level++;
    game_state_.set_game_level(level);
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
    someone_moved_ = true;

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
      (next != GetOppositeDirection(obj->direction()) &&
       MoveIfAvailable(obj, next, game_state_.game_map())) ||
      MoveIfAvailable(obj, obj->direction(), game_state_.game_map()) ||
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
      (GetOppositeDirection(obj->direction()) != Direction::UP &&
       MoveIfAvailable(obj, Direction::UP, game_state_.game_map())) ||
      (GetOppositeDirection(obj->direction()) != Direction::RIGHT &&
       MoveIfAvailable(obj, Direction::RIGHT, game_state_.game_map())) ||
      (GetOppositeDirection(obj->direction()) != Direction::DOWN &&
       MoveIfAvailable(obj, Direction::DOWN, game_state_.game_map())) ||
      (GetOppositeDirection(obj->direction()) != Direction::LEFT &&
       MoveIfAvailable(obj, Direction::LEFT, game_state_.game_map()));
    }
    obj->set_next_ticks(
        current_ticks +
        kGhostTicks[obj->index() % 4][static_cast<int>(obj->vitality())]);
  }

  void MoveLambdaMan(LambdaMan* obj, int current_ticks) {
    if (obj->next_ticks() > current_ticks) {
      return;
    }
    someone_moved_ = true;

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
      game_state_.mutable_lambda_man()->set_vitality(127 * 20);
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

  void PrepareForNextLevel() {
    game_state_.set_score(0);
    game_state_.set_game_level(game_state_.game_level() + 1);
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
  std::string lambdaman_file_;
  std::vector<std::string> ai_file_list_;
  bool someone_moved_;

  GameState game_state_;

  void PrintGame(int current_ticks) const {
    std::stringstream buffer;
    buffer << "\x1b[H";
    buffer << "level:" << game_state_.game_level()
           << "  score:" << game_state_.score() << "       \n";
    buffer << "ticks:" << current_ticks
           << "  life:" << game_state_.lambda_man().life()
           << "  vital: " << game_state_.lambda_man().vitality() << "      \n";

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
    puts(str.c_str());
  }
  // TODO event optimization.

  DISALLOW_COPY_AND_ASSIGN(Simulator);
};

void inthandler(int s) {
  printf("\e[?25h");
  exit(1);
}

// Set sigint handler to restore cursor.
void sethandler() {
  struct sigaction sig_int_handler;
  sig_int_handler.sa_handler = inthandler;
  sigemptyset(&sig_int_handler.sa_mask);
  sig_int_handler.sa_flags = 0;
  sigaction(SIGINT, &sig_int_handler, NULL);
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc < 3 || 7 < argc) {
    fprintf(stderr, "Usage: %s MAP LAMBDA AI1 [AI2 [AI3 [AI4]]]\n", argv[0]);
    return 2;
  }
  sethandler();
  Simulator sim;
  sim.set_map_file(argv[1]);
  sim.set_lambdaman_file(argv[2]);
  for (int i = 3; i < argc; ++i) {
    sim.add_ai_file(argv[i]);
  }
  sim.Run(true);
  printf("\e[?25h");
  return 0;
}
