#include <benchmark/benchmark.h>

#include "vibe-ecs.h"

static void BM_Saudacao(benchmark::State& state) {
    for (auto _ : state) {
        vibe_version();
    }
}

BENCHMARK(BM_Saudacao);

BENCHMARK_MAIN();
