#include <benchmark/benchmark.h>
#include "kv/Database.h"
#include <cstdio>
#include <string>

namespace {
    std::string tmpPath(const std::string& name) {
        return "bench_" + name + ".kv";
    }
    void cleanup(const std::string& path) {
        std::remove(path.c_str());
    }
}


static void BM_Set(benchmark::State& state) {
    const std::string path = tmpPath("set");
    cleanup(path);
    {
        kv::Database db(path);
        int counter = 0;
        for (auto _ : state) {
            db.Set("key" + std::to_string(counter++), "value");
        }
    }
    cleanup(path);
}
BENCHMARK(BM_Set);


static void BM_Get(benchmark::State& state) {
    const std::string path = tmpPath("get");
    cleanup(path);
    {
        kv::Database db(path);
        for (int i = 0; i < 10000; ++i) {
            db.Set("key" + std::to_string(i), "value" + std::to_string(i));
        }

        int i = 0;
        for (auto _ : state) {
            auto v = db.Get("key" + std::to_string(i++ % 10000));
            benchmark::DoNotOptimize(v);
        }
    }
    cleanup(path);
}
BENCHMARK(BM_Get);


static void BM_Delete(benchmark::State& state) {
    const std::string path = tmpPath("delete");
    cleanup(path);
    {
        kv::Database db(path);
        const int N = 10000;
        for (int i = 0; i < N; ++i) {
            db.Set("key" + std::to_string(i), "v");
        }
        int i = 0;
        for (auto _ : state) {
            if (i >= N) {
                db.Compact();
                for (int j = 0; j < N; ++j) {
                    db.Set("key" + std::to_string(j), "v");
                }
                i = 0;
            }
            db.Delete("key" + std::to_string(i++));
        }
    }
    cleanup(path);
}
BENCHMARK(BM_Delete);


static void BM_Compact(benchmark::State& state) {
    const std::string path = tmpPath("compact");
    cleanup(path);
    {
        kv::Database db(path);
        const int N = state.range(0);
        for (auto _ : state) {
            for (int i = 0; i < N; ++i) {
                db.Set("key", std::to_string(i));
            }
            db.Compact();
        }
    }
    cleanup(path);
}
BENCHMARK(BM_Compact)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Recovery(benchmark::State& state) {
    const std::string path = tmpPath("recovery");
    cleanup(path);
    const int N = state.range(0);

    {
        kv::Database db(path);
        for (int i = 0; i < N; ++i) {
            db.Set("key" + std::to_string(i), "value" + std::to_string(i));
        }
        db.Sync();
    }

    for (auto _ : state) {
        kv::Database db(path);
        benchmark::DoNotOptimize(db.size());
    }

    cleanup(path);
}
BENCHMARK(BM_Recovery)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();