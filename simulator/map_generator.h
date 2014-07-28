#ifndef SIMULATOR_MAP_GENERATOR_H_
#define SIMULATOR_MAP_GENERATOR_H_

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <algorithm>

//                       up  right down left
static const int DR[] = {-1,   0,   1,   0};
static const int DC[] = { 0,   1,   0,  -1};

class MapGenerator {
 public:
  MapGenerator() {
    empty_space_rate_ = 0.6;
    pill_rate_ = 0.4;
    power_pill_rate_ = 0.01;
    ghost_rate_ = 0.01;
  }
  
  void SetSeed(unsigned int seed) { srand(seed); }
  void SetEmptySpaceRate(double rate) { empty_space_rate_ = rate; }
  void SetPillRate(double rate) { pill_rate_ = rate; }
  void SetPowerPillRate(double rate) { power_pill_rate_ = rate; }
  void SetGhostRate(double rate) { ghost_rate_ = rate; }
  
  std::vector<std::string> Generate(int level) {
    int W = floor(sqrt(100 * level));
    int H = W;
    
    std::vector<std::string> cell(H, std::string(W, '#'));
    DigMap(cell);
    std::pair<int,int> lambda_pos = PlaceLambdaMan(cell);
    PlaceFruitLocation(cell);
    PlacePowerPill(cell);
    PlaceGhosts(cell, lambda_pos);
    PlacePills(cell);
    
    return cell;
  }
  
 private:
  double empty_space_rate_;
  double power_pill_rate_;
  double pill_rate_;
  double ghost_rate_;

  static bool IsInner(int r, int c, const std::vector<std::string>& cell) {
    return r > 0 && r < cell.size() - 1 &&
           c > 0 && c < cell[0].size() - 1;
  }
  
  static bool TryDig(int r, int c, std::vector<std::string>& cell) {
    cell[r][c] = ' ';
    for (int top = r-1; top <= r; top++)
      for (int left = c-1; left <= c; left++)
        if (cell[top][left] == ' ' && cell[top][left+1] == ' ' &&
            cell[top+1][left] == ' ' && cell[top+1][left+1] == ' ') {
          cell[r][c] = '#';
          return false;
        }
    return true;
  }
  
  static std::pair<int, int> GetEmptyLocation(std::vector<std::string>& cell) {
    while (true) {
      int r = 1 + rand() % (cell.size() - 2);
      int c = 1 + rand() % (cell[0].size() - 2);
      if (cell[r][c] == ' ') {
        return std::make_pair(r, c);
      }
    }
    return std::pair<int,int>();
  }

  std::pair<int,int> PlaceLambdaMan(std::vector<std::string>& cell) {
    std::pair<int,int> loc = GetEmptyLocation(cell);
    cell[loc.first][loc.second] = '\\';
    return loc;
  }
  
  void PlaceFruitLocation(std::vector<std::string>& cell) {
    std::pair<int,int> loc = GetEmptyLocation(cell);
    cell[loc.first][loc.second] = '%';
  }
  
  void PlacePowerPill(std::vector<std::string>& cell) {
    int num = floor((cell.size() - 2) * (cell[0].size() - 2) * power_pill_rate_);
    while (num--) {
      std::pair<int,int> loc = GetEmptyLocation(cell);
      cell[loc.first][loc.second] = 'o';
    }
  }
  
  void PlaceGhosts(std::vector<std::string>& cell, std::pair<int,int> lambda_pos) {
    int num = floor((cell.size() - 2) * (cell[0].size() - 2) * ghost_rate_);
    while (num) {
      std::pair<int,int> loc = GetEmptyLocation(cell);
      if (abs(loc.first - lambda_pos.first) > 3 || abs(loc.second - lambda_pos.second) > 3) {
        cell[loc.first][loc.second] = '=';
        num--;
      }
    }
  }
  
  void PlacePills(std::vector<std::string>& cell) {
    int num = floor((cell.size() - 2) * (cell[0].size() - 2) * pill_rate_);
    std::vector<std::pair<int,int> > empty_cells;
    for (int i = 0; i < cell.size(); i++)
      for (int j = 0; j < cell[0].size(); j++)
        if (cell[i][j] == ' ')
          empty_cells.push_back(std::make_pair(i, j));
    random_shuffle(empty_cells.begin(), empty_cells.end());
    for (int i = 0; i < num; i++)
      cell[empty_cells[i].first][empty_cells[i].second] = '.';
  }
  
  void DigMap(std::vector<std::string>& cell) {
    int H = cell.size();
    int W = cell[0].size();
    static const int SLOT[] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 2};
    int r = 1 + rand() % (H-2);
    int c = 1 + rand() % (W-2);
    int dir = rand() % 4;
    cell[r][c] = ' ';
    
    int digged = 0;
    int staying = 0;
    while (digged < (H-2) * (W-2) * empty_space_rate_) {
      int ndir = (dir + SLOT[rand() % (sizeof(SLOT) / sizeof(SLOT[0]))]) % 4;
      int nr = r + DR[ndir];
      int nc = c + DC[ndir];
      staying++;
      if (IsInner(nr, nc, cell) && cell[nr][nc] == '#' && TryDig(nr, nc, cell)) {
        r = nr;
        c = nc;
        dir = ndir;
        digged++;
        staying = 0;
      }
      if (staying == 10) {
        while (true) {
          r = 1 + rand() % (H-2);
          c = 1 + rand() % (W-2);
          dir = rand() % 4;
          if (cell[r][c] == ' ')
            break;
        }
        staying = 0;
      }
    }
  }
};



#endif
