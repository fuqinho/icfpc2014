virtual Direction GetNextDirection(const GameState& state) override {
  static const int DR[] = {0, 0, -1, 1};
  static const int DC[] = {-1, 1, 0, 0};
  static const int INF = 1000000000;

  size_t H = state.map_height();
  size_t W = state.map_width();
  std::vector<std::vector<int>> dist(H, std::vector<int>(W, INF));

  std::queue<std::pair<int,int>> que;
  int sr = (int)state.lambda_man().position().y;
  int sc = (int)state.lambda_man().position().x;
  que.push(std::make_pair(sr, sc));
  dist[sr][sc] = 0;

  int tr = -1, tc = -1;
  while (!que.empty()) {
    int r = que.front().first;
    int c = que.front().second;
    que.pop();
    for (int i = 0; i < 4; i++) {
      int nr = r + DR[i];
      int nc = c + DC[i];
      if (nr >= 0 && nr < H && nc >= 0 && nc < W && dist[nr][nc] == INF) {
        if (state.game_map()[nr][nc] != '#') {
          dist[nr][nc] = dist[r][c] + 1;
          que.push(std::make_pair(nr, nc));
        }
        if ((state.fruit() > 0 && state.game_map()[nr][nc] == '%') || state.game_map()[nr][nc] == '.' || state.game_map()[nr][nc] == 'o') {
          tr = nr;
          tc = nc;
          que = std::queue<std::pair<int,int>>();
          break;
        }
      }
    }
  }
  assert(tr >= 0 && tc >= 0);
  while (dist[tr][tc] > 1) {
    for (int i = 0; i < 4; i++) {
      int nr = tr + DR[i];
      int nc = tc + DC[i];
      if (nr >= 0 && nr < H && nc >= 0 && nc < W && dist[nr][nc] < dist[tr][tc]) {
        tr = nr;
        tc = nc;
        break;
      }
    }
  }
  if (tr == sr + 1 && tc == sc) return Direction::DOWN;
  if (tr == sr - 1 && tc == sc) return Direction::UP;
  if (tr == sr && tc == sc + 1) return Direction::RIGHT;
  if (tr == sr && tc == sc - 1) return Direction::LEFT;
  assert(0);
  return Direction::RIGHT;
}

