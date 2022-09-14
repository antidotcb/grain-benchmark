#include <benchmark/benchmark.h>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <sstream>
#include <stdexcept>
#include <vector>


template<typename T, typename C>
inline int loadGrainPass(const T levels, C compare) {
    int totalCapacity = 0;
    int currentCapacity = 0;
    int firstHeight = 0;

    for (const int currentHeight: levels) {
        if (compare(currentHeight, firstHeight))
            currentCapacity += firstHeight - currentHeight;
        else {
            firstHeight = currentHeight;
            totalCapacity += currentCapacity;
            currentCapacity = 0;
        }
    }
    return totalCapacity;
}

int loadGrain3(std::span<const int> levels) {
    return loadGrainPass(levels, std::less<>{}) + loadGrainPass(std::ranges::reverse_view(levels), std::less_equal<>{});
}

int loadGrain7(std::span<const int> levels) {
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


int loadGrain(const std::vector<int>& levels) {
    if (levels.size() < 3) return 0;
    using namespace std;
    int result = 0;
    size_t l, r, l_max, r_max;
    l = l_max = 0;
    r = r_max = static_cast<int>(levels.size() - 1);
    int max_r_level = levels[r];
    int r_level = max_r_level;
    int max_l_level = levels[l];
    int l_level = max_l_level;
    int area_l = 0;
    int area_r = 0;
    size_t len = 0;
    while (l < r - 1) {
        if (r_level <= l_level) {
            r--;
            r_level = levels[r];
            if (r_level < max_r_level) {
                area_r += r_level;
                continue;
            } else {
                len = r_max - r - 1;
                result += max_r_level * len - area_r;
                r_max = r;
                area_r = 0;
                max_r_level = r_level;
            }
        } else {
            l++;
            l_level = levels[l];
            if (l_level < max_l_level) {
                area_l += l_level;
                continue;
            } else {
                len = l - l_max - 1;
                result += max_l_level * len - area_l;
                l_max = l;
                area_l = 0;
                max_l_level = l_level;
            }
        }
    }
    len = r_max - l_max - 1;
    result += min(max_r_level, max_l_level) * len - area_r - area_l;
    return result;
}

static constexpr size_t g_Size = 100000000;
std::vector<int> vector(g_Size);

static void Setup() {
    std::srand(1);
    for (auto& i: vector) {
        i = std::rand() / (float) RAND_MAX * 10;
    }
}

static void BM_IanFirst(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        loadGrain3(vector);
    }
}
static void BM_IanSecond(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        loadGrain7(vector);
    }
}
static void BM_MyLatest(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        loadGrain(vector);
    }
}

BENCHMARK(BM_IanFirst);
BENCHMARK(BM_IanSecond);
BENCHMARK(BM_MyLatest);

BENCHMARK_MAIN();
