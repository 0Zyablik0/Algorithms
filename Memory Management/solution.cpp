#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <istream>
#include <iterator>
#include <list>
#include <ostream>
#include <random>
#include <utility>
#include <vector>

enum class RequestType {
  Allocate = 0,
  Free = 1,
};

struct MemoryRequest {
  RequestType type;
  size_t parameter;
};

struct MemoryResponse {
  int64_t data = 0;
};

struct Fragment {
  size_t begin = 0;
  size_t end = 0;
  size_t heap_num = 0;
  bool is_free = true;
  size_t Size() const { return end - begin; }
};

bool operator>(const Fragment &, const Fragment &);

class Heap {
  using HeapElem = std::list<Fragment>::iterator;

public:
  void Push(HeapElem it) {
    data.push_back(it);
    it->heap_num = data.size() - 1;
    SiftUp(data.size() - 1);
  }
  void Pop() {
    if (data.empty()) {
      return;
    }
    Delete(0);
  }

  void Delete(size_t index) {
    if (data.empty()) {
      return;
    }
    if (index != (data.size() - 1)) {
      Swap(index, data.size() - 1);
    }
    data.pop_back();
    if (index > 0 && *data[index] > *data[(index - 1) / 2]) {
      SiftUp(index);
    } else {
      SiftDown(index);
    }
  }

  HeapElem Top() { return data.front(); }
  size_t Size() const { return data.size(); }

private:
  void Swap(size_t lhs, size_t rhs) {
    std::swap(data[lhs], data[rhs]);
    data[lhs]->heap_num = lhs;
    data[rhs]->heap_num = rhs;
  }
  void SiftUp(size_t index) {
    if (data.empty()) {
      return;
    }
    size_t current = index;
    size_t parent = (current - 1) / 2;
    while ((current != 0)) {
      if (*data[current] > *data[parent]) {
        Swap(current, parent);
      } else {
        break;
      }
      current = parent;
      parent = (current - 1) / 2;
    }
  }
  void SiftDown(size_t current) {
    if (data.empty()) {
      return;
    }
    size_t left_child = 2 * current + 1;
    size_t right_child = 2 * current + 2;
    while (2 * current + 1 < data.size()) {
      size_t biggest_child = left_child;
      if (right_child < data.size() && *data[right_child] > *data[left_child]) {
        biggest_child = right_child;
      }
      if (*data[current] > *data[biggest_child]) {
        break;
      }
      Swap(current, biggest_child);
      current = biggest_child;
      left_child = 2 * current + 1;
      right_child = 2 * current + 2;
    }
  }
  std::vector<std::list<Fragment>::iterator> data;
};

class Memory {
public:
  explicit Memory(size_t size) : size_(size) {
    fragments_.push_back(Fragment{});
    fragments_.push_back(Fragment{0, size_});
    free_fragments_.Push(++fragments_.begin());
  }
  int64_t Allocate(size_t size) {
    if (free_fragments_.Size() == 0 || free_fragments_.Top()->Size() < size) {
      history_.push_back(fragments_.begin());
      return -1;
    }
    auto max_fragment = free_fragments_.Top();
    free_fragments_.Pop();
    if (max_fragment->Size() > size) {
      auto free_fragment = Fragment{
          .begin = max_fragment->begin + size,
          .end = max_fragment->end,
          .is_free = true,
      };
      fragments_.insert(std::next(max_fragment), free_fragment);
      free_fragments_.Push(std::next(max_fragment));
    }
    max_fragment->end = max_fragment->begin + size;
    max_fragment->is_free = false;
    history_.push_back(max_fragment);
    return static_cast<int64_t>(max_fragment->begin) + 1;
  }
  void Free(size_t index) {
    history_.push_back(fragments_.begin());
    auto fragment = history_[index];
    if (fragment == fragments_.begin()) {
      return;
    }
    history_[index] = fragments_.begin();
    auto previos = std::next(fragment, -1);
    auto next = std::next(fragment);
    fragment->is_free = true;
    if (previos != fragments_.begin() && previos->is_free) {
      free_fragments_.Delete(previos->heap_num);
      fragment->begin = previos->begin;
      fragments_.erase(previos);
    }
    if (next != fragments_.end() && next->is_free) {
      free_fragments_.Delete(next->heap_num);
      fragment->end = next->end;
      fragments_.erase(next);
    }
    free_fragments_.Push(fragment);
  }

  size_t Size() { return size_; }

private:
  size_t size_;

  std::list<Fragment> fragments_;
  std::vector<std::list<Fragment>::iterator> history_;
  Heap free_fragments_;
};

class MemoryManager {
public:
  explicit MemoryManager(size_t size) : memory_(Memory(size)){};

  std::vector<MemoryResponse>
  ServeRequests(const std::vector<MemoryRequest> &requests) {
    std::vector<MemoryResponse> responses;
    for (const auto &request : requests) {
      switch (request.type) {
      case RequestType::Allocate: {
        auto response =
            MemoryResponse{.data = memory_.Allocate(request.parameter)};
        responses.push_back(response);
        break;
      }
      case RequestType::Free: {
        memory_.Free(request.parameter);
        break;
      }
      };
    }
    return responses;
  }
  size_t Size() { return memory_.Size(); }

private:
  Memory memory_;
};

MemoryManager InitilizeManager(std::istream &stream);

std::vector<MemoryRequest> ReadRequest(std::istream &stream);

void PrintResponses(const std::vector<MemoryResponse> &responses,
                    std::ostream &stream);

void Test();

int main() {
  // Test();
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  auto manager = InitilizeManager(std::cin);
  auto requests = ReadRequest(std::cin);
  auto responses = manager.ServeRequests(requests);
  PrintResponses(responses, std::cout);
}

MemoryManager InitilizeManager(std::istream &stream) {
  size_t size = 0;
  stream >> size;
  return MemoryManager(size);
}

std::vector<MemoryRequest> ReadRequest(std::istream &stream) {
  size_t num_requests = 0;
  stream >> num_requests;
  std::vector<MemoryRequest> requests;
  for (size_t index = 0; index < num_requests; ++index) {
    int64_t data = 0;
    stream >> data;
    if (data > 0) {
      requests.push_back(MemoryRequest{
          .type = RequestType::Allocate,
          .parameter = static_cast<size_t>(data),
      });

    } else if (data < 0) {
      requests.push_back(MemoryRequest{
          .type = RequestType::Free,
          .parameter = static_cast<size_t>(-data) - 1,
      });
    }
  }
  return requests;
};

void PrintResponses(const std::vector<MemoryResponse> &responses,
                    std::ostream &stream) {
  for (const auto &response : responses) {
    stream << response.data << "\n";
  }
}

bool operator>(const Fragment &lhs, const Fragment &rhs) {
  if (lhs.Size() == rhs.Size()) {
    return lhs.begin < rhs.begin;
  }
  return lhs.Size() > rhs.Size();
}

// Tests

struct SlowFragment {
  size_t begin = 0;
  size_t end = 0;
  size_t Size() { return end - begin; }
};

class SlowMemoryManager {
public:
  explicit SlowMemoryManager(size_t size) {
    memory_ = std::vector<bool>(size);
    for (auto index = 0; index < size; ++index) {
      memory_[index] = true;
    }
  };

  std::vector<MemoryResponse>
  ServeRequests(const std::vector<MemoryRequest> &requests) {
    std::vector<MemoryResponse> responses;
    for (const auto &request : requests) {
      switch (request.type) {
      case RequestType::Allocate: {
        auto response = MemoryResponse{.data = Allocate(request.parameter)};
        responses.push_back(response);
        break;
      }
      case RequestType::Free: {
        Free(request.parameter);
        break;
      }
      };
    }
    return responses;
  }

private:
  int64_t Allocate(size_t size) {
    SlowFragment fragment = FindLargestFragment();
    if (fragment.Size() < size) {
      history_.push_back(SlowFragment{});
      return -1;
    }
    fragment.end = fragment.begin + size;
    for (auto index = fragment.begin; index < fragment.end; ++index) {
      memory_[index] = false;
    }
    history_.push_back(fragment);
    return fragment.begin + 1;
  }

  void Free(size_t index) {
    history_.push_back(SlowFragment{});
    auto fragment = history_[index];
    for (auto index = fragment.begin; index < fragment.end; ++index) {
      memory_[index] = true;
    }
  }

  SlowFragment FindLargestFragment() {
    SlowFragment result;
    size_t begin = 0;
    size_t end = 0;
    for (size_t index = 0; index < memory_.size(); ++index) {
      if (memory_[index]) {
        if (!memory_[begin]) {
          begin = index;
          end = index;
        }
        ++end;
      } else {
        if (begin != end) {
          if (result.Size() < (end - begin)) {
            result.begin = begin;
            result.end = end;
          }
        }
        begin = index;
        end = index;
      }
    }
    if (begin != end) {
      if (result.Size() < (end - begin)) {
        result.begin = begin;
        result.end = end;
      }
    }

    return result;
  }

  std::vector<SlowFragment> history_;
  std::vector<bool> memory_;
};

std::vector<MemoryRequest> GenRandomArray(std::mt19937 *gen, size_t count,
                                          size_t from, size_t to) {

  float eps = 0.3;
  std::uniform_real_distribution<double> probs(0.0, 1.0);

  std::vector<MemoryRequest> data(count);
  std::vector<size_t> allocated;
  size_t index = 0;
  for (MemoryRequest &cur : data) {
    auto prob = probs(*gen);

    if (prob < eps && !allocated.empty()) {
      std::uniform_int_distribution<size_t> dist(0, allocated.size() - 1);
      size_t parameter = dist(*gen);
      cur = MemoryRequest{
          .type = RequestType::Free,
          .parameter = allocated[parameter],
      };
      allocated.erase(allocated.begin() + parameter);
    } else {
      std::uniform_int_distribution<size_t> dist(from, to);
      cur = MemoryRequest{
          .type = RequestType::Allocate,

          .parameter = dist(*gen),
      };
      allocated.push_back(index);
    }
    ++index;
  }
  return data;
}

void StressTest() {
  std::mt19937 generator(7274);
  const size_t max_size = 100;
  const size_t count = 10000;
  std::uniform_int_distribution<size_t> size_dist(10, max_size);
  for (int iter = 1; iter <= 100; ++iter) {
    std::cerr << "Test " << iter << "... "
              << "\r";
    size_t size = size_dist(generator);
    auto requests = GenRandomArray(&generator, count, 1, max_size);
    auto fast_manager = MemoryManager(size);
    auto slow_manager = SlowMemoryManager(size);
    auto fast_responses = fast_manager.ServeRequests(requests);
    auto slow_responses = slow_manager.ServeRequests(requests);
    assert(fast_responses.size() == slow_responses.size());
    for (size_t index = 0; index < fast_responses.size(); ++index) {
      assert(fast_responses[index].data == slow_responses[index].data);
    }
  }
  std::cerr << "\n";
}

void Test() { StressTest(); }
