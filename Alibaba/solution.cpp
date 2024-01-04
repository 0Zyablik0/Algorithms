#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <istream>
#include <optional>
#include <vector>

struct Coin {
  size_t distance;
  int64_t time;
};

struct Ending {
  int64_t left = -1;
  int64_t right = -1;
};

std::vector<Coin> ReadCoins(std::istream &stream);

std::optional<int64_t> FindOptimalTime(const std::vector<Coin> &coins);

bool operator<(const Coin &lhs, const Coin &rhs);

int main() {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);
  auto coins = ReadCoins(std::cin);
  std::sort(coins.begin(), coins.end());
  auto result = FindOptimalTime(coins);
  if (result.has_value()) {
    std::cout << result.value() << "\n";
  } else {
    std::cout << "No solution"
              << "\n";
  }
  return 0;
}

std::vector<Coin> ReadCoins(std::istream &stream) {
  size_t size = 0;
  stream >> size;

  auto coins = std::vector<Coin>{size};
  for (size_t index = 0; index < size; ++index) {
    auto &coin = coins[index];
    stream >> coin.distance >> coin.time;
  }

  return coins;
}

bool operator<(const Coin &lhs, const Coin &rhs) {
  return lhs.distance < rhs.distance;
}

typedef std::vector<std::vector<Ending>> DP;

DP InitializeDP(size_t size) {
  std::vector<std::vector<Ending>> solutions{size};
  for (size_t index = 0; index < size; ++index) {
    solutions[index] = std::vector<Ending>{size};
    solutions[index][index].right = 0;
    solutions[index][index].left = 0;
  }
  return solutions;
}

void SolveDPLeft(DP *solutions, const std::vector<Coin> &coins, size_t left,
                 size_t right) {
  auto &result = *solutions;
  if (result[left + 1][right].left >= 0) {
    int64_t distance = coins[left + 1].distance - coins[left].distance;
    int64_t total_time = result[left + 1][right].left + distance;
    if (total_time <= coins[left].time) {
      result[left][right].left = total_time;
    }
  }
  if (result[left + 1][right].right >= 0) {
    int64_t distance = coins[right].distance - coins[left].distance;
    int64_t total_time = result[left + 1][right].right + distance;
    if (total_time <= coins[left].time) {
      int64_t current_time = result[left][right].left;
      if (current_time == -1 || current_time > total_time) {
        result[left][right].left = total_time;
      }
    }
  }
}

void SolveDPRight(DP *solutions, const std::vector<Coin> &coins, size_t left,
                  size_t right) {
  auto &result = *solutions;
  if (result[left][right - 1].right >= 0) {
    int64_t distance = coins[right].distance - coins[right - 1].distance;
    int64_t total_time = result[left][right - 1].right + distance;
    if (total_time <= coins[right].time) {
      result[left][right].right = total_time;
    }
  }
  if (result[left][right - 1].left >= 0) {
    int64_t distance = coins[right].distance - coins[left].distance;
    int64_t total_time = result[left][right - 1].left + distance;
    if (total_time <= coins[right].time) {
      int64_t current_time = result[left][right].right;
      if (current_time == -1 || current_time > total_time) {
        result[left][right].right = total_time;
      }
    }
  }
}

std::optional<int64_t> FindOptimalTime(const std::vector<Coin> &coins) {
  if (coins.empty()) {
    return {};
  }
  size_t max_left = 0;
  size_t max_right = coins.size();
  size_t max_degree = max_right - max_left;
  DP solutions = InitializeDP(coins.size());

  for (size_t degree = 1; degree < max_degree; ++degree) {
    for (size_t right = degree; right < max_right; ++right) {
      size_t left = right - degree;
      SolveDPLeft(&solutions, coins, left, right);
      SolveDPRight(&solutions, coins, left, right);
    }
  }

  int64_t total_time_left = solutions[0][max_right - 1].left;
  int64_t total_time_right = solutions[0][max_right - 1].right;
  if (total_time_left == -1) {
    if (total_time_right == -1) {
      return {};
    }
    return total_time_right;
  } else {
    if (total_time_right == -1) {
      return total_time_left;
    }
    return std::min(total_time_left, total_time_right);
  }
  return {};
}
