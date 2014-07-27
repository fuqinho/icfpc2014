#include "gamecommon.h"
#include "lambdaman.h"
#include "ghost.h"
#include "gamestate.h"

#define USE_TEST_AI 0

#if USE_TEST_AI
#include "testai.h"
#endif

LambdaMan::LambdaMan() {
}

LambdaMan::~LambdaMan() {
}

void LambdaMan::LoadProgram(const std::string& program) {
  gcc_.LoadProgram(program);
}

void LambdaMan::Init(const GameState& game_state) {
  Value result =
      gcc_.Run(0, -1, BuildWorld(game_state),
               Value {ValueTag::INT, 0}, Value {ValueTag::INT, 0});
    ai_state_ = gcc_.ConsCar(result);
  step_function_ = gcc_.ConsCdr(result);
}

Direction LambdaMan::GetNextDirection(const GameState& state) {
#if !USE_TEST_AI
  Value result =
      gcc_.Run(gcc_.GetIp(step_function_),
               gcc_.GetEnv(step_function_),
               ai_state_,
               BuildWorld(state),
               step_function_);
  ai_state_ = gcc_.ConsCar(result);
  Value direction = gcc_.ConsCdr(result);
  return static_cast<Direction>(direction.value);
#else
  return GetNextDirectionGreedy(state);
#endif
}

Value LambdaMan::BuildWorld(const GameState& game_state) {
  gcc_.RunFullGC(ai_state_, step_function_);
  Value m = BuildMap(game_state.game_map());
  Value lambda = BuildLambdaMan(game_state.lambda_man(), game_state.score());
  Value ghost_list = BuildGhostList(game_state.ghost_list());
  Value fruit = BuildFruit(game_state.fruit());
  return gcc_.Create4Tuple(m, lambda, ghost_list, fruit);
}

Value LambdaMan::BuildMap(const GameMap& game_map) {
  Value result = { ValueTag::INT, 0 };
  for (int y = game_map.size() - 1; y >= 0; --y) {
    Value line = { ValueTag::INT, 0 };
    for (int x = game_map[y].size() - 1; x >= 0; --x) {
      int v;
      switch (game_map[y][x]) {
        case '#': v = 0; break;
        case ' ': v = 1; break;
        case '.': v = 2; break;
        case 'o': v = 3; break;
        case '%': v = 4; break;
        case '\\': v = 5; break;
        case '=': v = 6; break;
        default:
          assert(false);
      }
      line = gcc_.Create2Tuple(Value {ValueTag::INT, v}, line);
    }
    result = gcc_.Create2Tuple(line, result);
  }
  return result;
}

Value LambdaMan::BuildLambdaMan(const LambdaMan& lambdaman, int score) {
  Value vitality = Value { ValueTag::INT, lambdaman.vitality() };
  Value location = gcc_.Create2Tuple(
      Value { ValueTag::INT, static_cast<int32_t>(lambdaman.position().x) },
      Value { ValueTag::INT, static_cast<int32_t>(lambdaman.position().y) });
  Value direction = Value {
    ValueTag::INT, static_cast<int>(lambdaman.direction()) };
  Value remaining_life = Value { ValueTag::INT, lambdaman.life() };
  Value score_v = Value { ValueTag::INT, score };
  return gcc_.Create5Tuple(
      vitality, location, direction, remaining_life, score_v);
}

Value LambdaMan::BuildGhostList(
    const std::vector<std::unique_ptr<Ghost> >& ghost_list) {
  Value result = Value { ValueTag::INT, 0 };
  for (int i = ghost_list.size() - 1; i >= 0; --i) {
    result = gcc_.Create2Tuple(BuildGhost(*ghost_list[i]), result);
  }
  return result;
}

Value LambdaMan::BuildGhost(const Ghost& ghost) {
  Value vitality = Value {
    ValueTag::INT, static_cast<int32_t>(ghost.vitality()) };
  Value location = gcc_.Create2Tuple(
      Value { ValueTag::INT, static_cast<int32_t>(ghost.position().x) },
      Value { ValueTag::INT, static_cast<int32_t>(ghost.position().y) });
  Value direction = Value {
    ValueTag::INT, static_cast<int32_t>(ghost.direction()) };
  return gcc_.Create3Tuple(vitality, location, direction);
}

Value LambdaMan::BuildFruit(int fruit) {
  return { ValueTag::INT, fruit };
}
