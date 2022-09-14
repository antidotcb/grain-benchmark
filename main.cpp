#include <benchmark/benchmark.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <sstream>
#include <stdexcept>
#include <vector>


int loadGrainIan(std::span<const int> levels) {
    if (levels.size() < 2)
        return 0;

    auto levelsBegin = levels.begin();
    auto levelsEnd = levels.end() - 1;
    bool forward = true;

    int totalCapacity = 0;

    int firstHeight = 0;
    int lastHeight = *levelsEnd;

    for (; levelsBegin != levelsEnd; forward ? ++levelsBegin : --levelsBegin) {
        const int currentHeight = *levelsBegin;
        if (currentHeight <= firstHeight)
            totalCapacity += firstHeight - currentHeight;
        else {
            firstHeight = currentHeight;
            if (firstHeight > lastHeight) {
                std::swap(firstHeight, lastHeight);
                std::swap(levelsBegin, levelsEnd);
                forward = !forward;
            }
        }
    }

    return totalCapacity;
}

inline void calculate(const std::vector<int>& levels, size_t pos, size_t& pos_max, int& max_level, int& level, size_t& area, size_t& result) {
    level = levels[pos];
    if (level < max_level) {
        area += level;
    } else {
        size_t len = pos_max - pos - 1;
        result += max_level * len - area;
        pos_max = pos;
        area = 0;
        max_level = level;
    }
}

size_t loadGrainMe(const std::vector<int>& levels) {
    if (levels.size() < 3) return 0;
    using namespace std;
    size_t result = 0;
    size_t l, r, l_max, r_max;
    l = l_max = 0;
    r = r_max = static_cast<int>(levels.size() - 1);
    int max_r_level = levels[r];
    int r_level = max_r_level;
    int max_l_level = levels[l];
    int l_level = max_l_level;
    size_t area_l = 0;
    size_t area_r = 0;
    while (l < r - 1) {
        if (r_level <= l_level) {
            r--;
            calculate(levels, r, r_max, max_r_level, r_level, area_r, result);
        } else {
            l++;
            calculate(levels, l, l_max, max_l_level, l_level, area_l, result);
        }
    }
    auto len = r_max - l_max - 1;
    result += min(max_r_level, max_l_level) * len - area_r - area_l;
    return result;
}


static constexpr size_t g_Size = 100000000;
static constexpr int g_Max = 100000;
std::vector<int> vector(g_Size);

static void Setup() {
    std::srand(std::chrono::steady_clock::now().time_since_epoch().count());
    for (auto& i: vector) {
        i = static_cast<int>(std::round((std::rand() / (float) RAND_MAX) * g_Max));
    }
}

static void BM_ian(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        benchmark::DoNotOptimize(loadGrainIan(vector));
    }
}

static void MB_antidotcb(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        benchmark::DoNotOptimize(loadGrainMe(vector));
    }
}

BENCHMARK(BM_ian);
BENCHMARK(MB_antidotcb);

BENCHMARK_MAIN();
