#include <benchmark/benchmark.h>

#include <vector>

#include "archetype-chunk.hpp"
#include "entity.hpp"
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
  state.SetItemsProcessed(state.iterations() * set.size());
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
  state.SetItemsProcessed(state.iterations() * set.size());
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
  state.SetItemsProcessed(state.iterations() * data.size());
}

BENCHMARK(BM_Baseline_Array_Iteration)->Range(100, 100000);

struct BenchPosComponent {
  uint64_t m_id;
  float m_x, m_y;

  BenchPosComponent() : m_id(0), m_x(0), m_y(0) {}
  explicit BenchPosComponent(uint64_t id) : m_id(id), m_x(0), m_y(0) {}
};

struct BenchVelComponent {
  uint64_t m_id;
  float m_vx, m_vy;

  BenchVelComponent() : m_id(0), m_vx(0), m_vy(0) {}
  explicit BenchVelComponent(uint64_t id) : m_id(id), m_vx(0), m_vy(0) {}
};

static void BM_ArchetypeChunk_Iteration_ForEach(benchmark::State &state) {
  size_t n = state.range(0);
  ArchetypeChunk<BenchPosComponent, BenchVelComponent> chunk;

  for (size_t i = 0; i < n; ++i) {
    Entity e(i + 1);
    chunk.add_entity(e);
    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    chunk.set_component(e, pos);
    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    chunk.set_component(e, vel);
  }

  float sum_x = 0;
  for (auto _ : state) {
    chunk.for_each_entity([&sum_x](Entity e) { sum_x += 1.0f; });
  }

  benchmark::DoNotOptimize(sum_x);
  state.SetItemsProcessed(state.iterations() * chunk.size());
}

BENCHMARK(BM_ArchetypeChunk_Iteration_ForEach)->Range(100, 100000);

static void BM_ArchetypeChunk_Iteration_RangeFor(benchmark::State &state) {
  size_t n = state.range(0);
  ArchetypeChunk<BenchPosComponent, BenchVelComponent> chunk;

  for (size_t i = 0; i < n; ++i) {
    Entity e(i + 1);
    chunk.add_entity(e);
    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    chunk.set_component(e, pos);
    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    chunk.set_component(e, vel);
  }

  float sum_x = 0;
  for (auto _ : state) {
    for (Entity e : chunk) {
      auto *pos = chunk.get_component<BenchPosComponent>(e);
      if (pos)
        sum_x += pos->m_x;
    }
  }

  benchmark::DoNotOptimize(sum_x);
  state.SetItemsProcessed(state.iterations() * chunk.size());
}

BENCHMARK(BM_ArchetypeChunk_Iteration_RangeFor)->Range(100, 100000);

static void BM_ArchetypeChunk_Iteration_AllComponents(benchmark::State &state) {
  size_t n = state.range(0);
  ArchetypeChunk<BenchPosComponent, BenchVelComponent> chunk;

  for (size_t i = 0; i < n; ++i) {
    Entity e(i + 1);
    chunk.add_entity(e);
    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    chunk.set_component(e, pos);
    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    chunk.set_component(e, vel);
  }

  float sum = 0;
  for (auto _ : state) {
    for (Entity e : chunk) {
      auto *pos = chunk.get_component<BenchPosComponent>(e);
      auto *vel = chunk.get_component<BenchVelComponent>(e);
      if (pos && vel) {
        sum += pos->m_x + pos->m_y + vel->m_vx + vel->m_vy;
      }
    }
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * chunk.size());
}

BENCHMARK(BM_ArchetypeChunk_Iteration_AllComponents)->Range(100, 100000);

static void BM_ArchetypeChunk_DirectArray_Iteration(benchmark::State &state) {
  size_t n = state.range(0);
  std::vector<Entity> entities;
  std::vector<BenchPosComponent> positions;
  std::vector<BenchVelComponent> velocities;
  entities.reserve(n);
  positions.reserve(n);
  velocities.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    entities.push_back(Entity(i + 1));
    positions.push_back(BenchPosComponent(i + 1));
    positions.back().m_x = static_cast<float>(i);
    positions.back().m_y = static_cast<float>(i * 2);
    velocities.push_back(BenchVelComponent(i + 1));
    velocities.back().m_vx = static_cast<float>(i * 3);
    velocities.back().m_vy = static_cast<float>(i * 4);
  }

  float sum = 0;
  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      sum += positions[i].m_x + positions[i].m_y + velocities[i].m_vx +
             velocities[i].m_vy;
    }
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * entities.size());
}

#include "archetype.hpp"

static void BM_Archetype_CreateEntity(benchmark::State &state) {
  size_t n = state.range(0);

  for (auto _ : state) {
    Archetype<BenchPosComponent, BenchVelComponent> archetype;
    for (size_t i = 0; i < n; ++i) {
      archetype.create_entity();
    }
  }

  state.SetItemsProcessed(state.iterations() * n);
}

BENCHMARK(BM_Archetype_CreateEntity)->Range(100, 100000);

static void BM_Archetype_SetGetComponent(benchmark::State &state) {
  size_t n = state.range(0);
  Archetype<BenchPosComponent, BenchVelComponent> archetype;

  std::vector<Entity> entities;
  entities.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    Entity e = archetype.create_entity();
    entities.push_back(e);

    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    archetype.set_component(e, pos);

    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    archetype.set_component(e, vel);
  }

  float sum = 0;
  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      auto *pos = archetype.get_component<BenchPosComponent>(entities[i]);
      auto *vel = archetype.get_component<BenchVelComponent>(entities[i]);
      if (pos && vel) {
        sum += pos->m_x + vel->m_vx;
      }
    }
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * archetype.entity_count());
}

BENCHMARK(BM_Archetype_SetGetComponent)->Range(100, 100000);

static void BM_Archetype_ForEachEntity(benchmark::State &state) {
  size_t n = state.range(0);
  Archetype<BenchPosComponent, BenchVelComponent> archetype;

  for (size_t i = 0; i < n; ++i) {
    Entity e = archetype.create_entity();

    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    archetype.set_component(e, pos);

    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    archetype.set_component(e, vel);
  }

  float sum = 0;
  for (auto _ : state) {
    archetype.for_each_entity([&sum, &archetype](Entity e) {
      auto *pos = archetype.get_component<BenchPosComponent>(e);
      if (pos)
        sum += pos->m_x;
    });
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * archetype.entity_count());
}

BENCHMARK(BM_Archetype_ForEachEntity)->Range(100, 100000);

static void BM_Archetype_Iteration(benchmark::State &state) {
  size_t n = state.range(0);
  Archetype<BenchPosComponent, BenchVelComponent> archetype;

  for (size_t i = 0; i < n; ++i) {
    Entity e = archetype.create_entity();

    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    archetype.set_component(e, pos);

    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    archetype.set_component(e, vel);
  }

  float sum = 0;
  for (auto _ : state) {
    archetype.for_each_entity([&sum, &archetype](Entity e) {
      auto *pos = archetype.get_component<BenchPosComponent>(e);
      auto *vel = archetype.get_component<BenchVelComponent>(e);
      if (pos && vel) {
        sum += pos->m_x + pos->m_y + vel->m_vx + vel->m_vy;
      }
    });
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * archetype.entity_count());
}

BENCHMARK(BM_Archetype_Iteration)->Range(100, 100000);

static void BM_Archetype_ForEach(benchmark::State &state) {
  size_t n = state.range(0);
  Archetype<BenchPosComponent, BenchVelComponent> archetype;

  for (size_t i = 0; i < n; ++i) {
    Entity e = archetype.create_entity();

    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    archetype.set_component(e, pos);

    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    archetype.set_component(e, vel);
  }

  float sum = 0;
  for (auto _ : state) {
    archetype.for_each([&sum](Entity e, const BenchPosComponent &pos,
                              const BenchVelComponent &vel) {
      sum += pos.m_x + pos.m_y + vel.m_vx + vel.m_vy;
    });
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * archetype.entity_count());
}

BENCHMARK(BM_Archetype_ForEach)->Range(100, 100000);

static void BM_Archetype_ForEach_SingleComponent(benchmark::State &state) {
  size_t n = state.range(0);
  Archetype<BenchPosComponent> archetype;

  for (size_t i = 0; i < n; ++i) {
    Entity e = archetype.create_entity();

    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    archetype.set_component(e, pos);
  }

  float sum = 0;
  for (auto _ : state) {
    archetype.for_each(
        [&sum](Entity e, const BenchPosComponent &pos) { sum += pos.m_x; });
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * archetype.entity_count());
}

BENCHMARK(BM_Archetype_ForEach_SingleComponent)->Range(100, 100000);

static void BM_Archetype_ForEach_Vs_Manual(benchmark::State &state) {
  size_t n = state.range(0);
  Archetype<BenchPosComponent, BenchVelComponent> archetype;

  for (size_t i = 0; i < n; ++i) {
    Entity e = archetype.create_entity();

    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    archetype.set_component(e, pos);

    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    archetype.set_component(e, vel);
  }

  float sum_for_each = 0;
  for (auto _ : state) {
    archetype.for_each([&sum_for_each](Entity e, const BenchPosComponent &pos,
                                       const BenchVelComponent &vel) {
      sum_for_each += pos.m_x + vel.m_vx;
    });
  }

  benchmark::DoNotOptimize(sum_for_each);
  state.SetItemsProcessed(state.iterations() * archetype.entity_count());
}

BENCHMARK(BM_Archetype_ForEach_Vs_Manual)->Range(100, 100000);

static void BM_ArchetypeChunk_ForEach(benchmark::State &state) {
  size_t n = state.range(0);
  ArchetypeChunk<BenchPosComponent, BenchVelComponent> chunk;

  for (size_t i = 0; i < n; ++i) {
    Entity e(i + 1);
    chunk.add_entity(e);

    BenchPosComponent pos(i + 1);
    pos.m_x = static_cast<float>(i);
    pos.m_y = static_cast<float>(i * 2);
    chunk.set_component(e, pos);

    BenchVelComponent vel(i + 1);
    vel.m_vx = static_cast<float>(i * 3);
    vel.m_vy = static_cast<float>(i * 4);
    chunk.set_component(e, vel);
  }

  float sum = 0;
  for (auto _ : state) {
    chunk.for_each([&sum](Entity e, const BenchPosComponent &pos,
                          const BenchVelComponent &vel) {
      sum += pos.m_x + pos.m_y + vel.m_vx + vel.m_vy;
    });
  }

  benchmark::DoNotOptimize(sum);
  state.SetItemsProcessed(state.iterations() * chunk.size());
}

BENCHMARK(BM_ArchetypeChunk_ForEach)->Range(100, 100000);

BENCHMARK(BM_ArchetypeChunk_DirectArray_Iteration)->Range(100, 100000);
