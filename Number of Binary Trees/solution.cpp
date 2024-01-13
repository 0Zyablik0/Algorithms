#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <istream>
#include <random>
#include <vector>

const size_t kModule = 123456789;
std::vector<int64_t> ReadElements(std::istream &stream);

size_t CountBinaryTrees(const std::vector<int64_t> &elements);

void Test();

int main() {
  Test();
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);
  auto numbers = ReadElements(std::cin);
  std::sort(numbers.begin(), numbers.end());
  size_t count = CountBinaryTrees(numbers);
  std::cout << count << "\n";
}

std::vector<int64_t> ReadElements(std::istream &stream) {
  size_t size = 0;
  stream >> size;
  std::vector<int64_t> result(size);
  for (auto &elem : result) {
    stream >> elem;
  }
  return result;
}

size_t CountBinaryTrees(const std::vector<int64_t> &elements) {
  if (elements.empty()) {
    return 0;
  }
  size_t size = elements.size();
  std::vector<std::vector<size_t>> solution{size + 1};
  for (size_t left = 0; left <= size; ++left) {
    solution[left] = std::vector<size_t>(size + 1);
    for (size_t index = 2; index <= size; ++index) {
      solution[left][index] = 0;
    }
    solution[left][0] = 1;
    solution[left][1] = 1;
  }
  for (size_t index = 0; index <= size; ++index) {
    solution[size][index] = 1;
  }
  for (size_t index = 2; index <= size; ++index) {
    for (size_t left = 0; left + index <= size; ++left) {
      for (size_t right = left; right < left + index; ++right) {
        size_t right_index = left + index - right - 1;
        size_t left_index = right - left;
        if (right == left || elements[right - 1] != elements[right]) {
          solution[left][index] +=
              (solution[left][left_index] * solution[right + 1][right_index]) %
              kModule;
          solution[left][left_index] %= kModule;
        }
      }
    }
  }
  return solution[0][size] % kModule;
}

template <typename Iterator>
size_t SlowContBinaryTrees(Iterator begin, Iterator end) {
  if (begin == end) {
    return 1;
  }
  if (begin + 1 == end) {
    return 1;
  }
  size_t total = 0;
  Iterator it = begin;
  while (it < end) {
    auto current_value = *it;
    auto left_total = SlowContBinaryTrees(begin, it) % kModule;
    auto right_total = SlowContBinaryTrees(it + 1, end) % kModule;
    total += (left_total * right_total) % kModule;
    while (it < end && *it == current_value) {
      ++it;
    }
  }
  return total % kModule;
}

std::vector<int64_t> GenRandomArray(std::mt19937 *gen, size_t count,
                                    int64_t from, int64_t to) {
  std::uniform_int_distribution<int64_t> dist(from, to);
  std::vector<int64_t> data(count);
  for (int64_t &cur : data) {
    cur = dist(*gen);
  }
  return data;
}

void StressTest() {
  std::mt19937 generator(72874);
  const int max_value = 5;
  const int max_size = 17;
  std::uniform_int_distribution<int64_t> dist(0, max_value);
  for (int iter = 1; iter <= 1000; ++iter) {
    std::cerr << "Test " << iter << "... ";
    auto data = GenRandomArray(&generator, max_size, 0, max_value);
    std::sort(data.begin(), data.end());
    auto fast_answer = CountBinaryTrees(data);
    auto slow_answer = SlowContBinaryTrees(data.begin(), data.end());
    if (fast_answer == slow_answer) {
      std::cerr << "OK\n";
    } else {
      std::cerr << "Fail\n";
    }
  }
}

void Test() { StressTest(); }
