#include <benchmark/benchmark.h>

#include "sparse-set.hpp"

struct BenchId {
  uint64_t m_id;

  BenchId() : m_id(0) {}
  explicit BenchId(uint64_t id) : m_id(id) {}

  operator uint64_t() const { return m_id; }
};

static void BM_SparseSet_Insert(benchmark::State &state) {
  size_t n = state.range(0);
  SparseSet<BenchId> set;

  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      set.insert(BenchId(i));
    }
  }

  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_SparseSet_Insert)->Range(100, 100000);

static void BM_SparseSet_Contains(benchmark::State &state) {
  size_t n = state.range(0);
  SparseSet<BenchId> set;

  for (size_t i = 0; i < n; ++i) {
    set.insert(BenchId(i));
  }

  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      benchmark::DoNotOptimize(set.contains(i));
    }
  }

  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_SparseSet_Contains)->Range(100, 100000);

static void BM_SparseSet_Iteration_ForEach(benchmark::State &state) {
  size_t n = state.range(0);
  SparseSet<BenchId> set;

  for (size_t i = 0; i < n; ++i) {
    set.insert(BenchId(i));
  }

  size_t sum = 0;
  for (auto _ : state) {
    set.for_each([&sum](const BenchId &id) { sum += id.m_id; });
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_SparseSet_Iteration_ForEach)->Range(100, 100000);

static void BM_SparseSet_Mixed(benchmark::State &state) {
  size_t n = state.range(0);
  SparseSet<BenchId> set;

  for (size_t i = 0; i < n; ++i) {
    set.insert(BenchId(i));
  }

  for (auto _ : state) {
    for (size_t i = 0; i < n / 2; ++i) {
      set.contains(i);
    }
    for (size_t i = 0; i < n / 2; ++i) {
      set.remove(i);
    }
    for (size_t i = n; i < n + n / 2; ++i) {
      set.insert(BenchId(i));
    }
  }

  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_SparseSet_Mixed)->Range(100, 100000);

static void BM_SparseSet_Iterator(benchmark::State &state) {
  size_t n = state.range(0);
  SparseSet<BenchId> set;

  for (size_t i = 0; i < n; ++i) {
    set.insert(BenchId(i));
  }

  size_t sum = 0;
  for (auto _ : state) {
    for (const auto &id : set) {
      sum += id.m_id;
    }
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_SparseSet_Iterator)->Range(100, 100000);

static void BM_SparseSet_Iterator_RangeFor(benchmark::State &state) {
  size_t n = state.range(0);
  SparseSet<BenchId> set;

  for (size_t i = 0; i < n; ++i) {
    set.insert(BenchId(i));
  }

  size_t sum = 0;
  for (auto _ : state) {
    for (const auto &id : set) {
      sum += id.m_id;
    }
    (void)set.size();
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_SparseSet_Iterator_RangeFor)->Range(100, 100000);

static void BM_Baseline_Array_Iteration(benchmark::State &state) {
  size_t n = state.range(0);
  std::vector<BenchId> data;
  data.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    data.push_back(BenchId(i));
  }

  size_t sum = 0;
  for (auto _ : state) {
    for (const auto &id : data) {
      sum += id.m_id;
    }
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_Baseline_Array_Iteration)->Range(100, 100000);
