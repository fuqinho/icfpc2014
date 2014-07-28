#include "mapstore.h"
#include "map_generator.h"
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

vector<string> CLASSIC = {
  "#######################",
  "#..........#..........#",
  "#.###.####.#.####.###.#",
  "#o###.####.#.####.###o#",
  "#.....................#",
  "#.###.#.#######.#.###.#",
  "#.....#....#....#.....#",
  "#####.#### # ####.#####",
  "#   #.#    =    #.#   #",
  "#####.# ### ### #.#####",
  "#    .  # === #  .    #",
  "#####.# ####### #.#####",
  "#   #.#    %    #.#   #",
  "#####.# ####### #.#####",
  "#..........#..........#",
  "#.###.####.#.####.###.#",
  "#o..#......\\......#..o#",
  "###.#.#.#######.#.#.###",
  "#.....#....#....#.....#",
  "#.########.#.########.#",
  "#.....................#",
  "#######################"
};

vector<string> GetMap(string id) {
  if (id == "classic")
    return CLASSIC;
  if (id.substr(0, 6) == "random") {
    int level = 5;
    if (id.length() > 6)
      level = atoi(id.substr(6).c_str());
    MapGenerator generator;
    return generator.Generate(level);
  }
  return vector<string>();
}

