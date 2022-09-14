#include <benchmark/benchmark.h>
#include <cassert>
#include <cmath>
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

int loadGrain2(const std::vector<int>& levels) {
    const auto size = levels.size();
    std::vector<int> left(size + 1, 0);
    std::vector<int> right(size + 1, 0);
    left[0] = 0;
    right[size] = 0;
    int result = 0;

    for (auto i = 0; i < size; ++i) {
        const auto j = size - i - 1;
        left[i + 1] = std::max(levels[i], left[i]);
        right[j] = std::max(levels[j], right[j + 1]);
        const auto grain_1 = std::max(std::min(left[i], right[i + 1]) - levels[i], 0);
        const auto grain_2 = std::max(std::min(left[j], right[j + 1]) - levels[j], 0);

        result += (i >= size / 2) ? (i == j ? grain_1 : grain_1 + grain_2) : 0;
    }

    return result;
}

int loadGrain(const std::vector<int>& levels) {
    if (levels.size() < 3) return 0;
    using namespace std;
    int result = 0;
    int l, r, l_max, r_max;
    l = l_max = 0;
    r = r_max = static_cast<int>(levels.size() - 1);
    int max_r_level = levels[r];
    int r_level = max_r_level;
    int max_l_level = levels[l];
    int l_level = max_l_level;
    int area_l = 0;
    int area_r = 0;
    int len;
    while (l < r - 1) {
        if (r_level <= l_level) {
            r--;


            r_level = levels[r];
            if (r_level < max_r_level) {
                area_r += r_level;
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
static constexpr int g_Max = 10000000;
std::vector<int> vector(g_Size);

static void Setup() {
    std::srand(std::chrono::steady_clock::now().time_since_epoch().count());
    for (auto& i: vector) {
        i = static_cast<int>(std::round((std::rand() / (float) RAND_MAX) * g_Max));
    }
}

static void BM_IanFirst(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        benchmark::DoNotOptimize(loadGrain3(vector));
    }
}
static void BM_IanSecond(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        benchmark::DoNotOptimize(loadGrain7(vector));
    }
}
static void BM_MyFirst(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        benchmark::DoNotOptimize(loadGrain2(vector));
    }
}
static void BM_MyLatest(benchmark::State& state) {
    Setup();
    for (auto _: state) {
        benchmark::DoNotOptimize(loadGrain(vector));
    }
}

//BENCHMARK(BM_IanFirst);
BENCHMARK(BM_IanSecond);
//BENCHMARK(BM_MyFirst);
BENCHMARK(BM_MyLatest);

BENCHMARK_MAIN();
