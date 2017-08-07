#include <benchmark/benchmark.h>
#include "flatmap.hpp"
#include <map>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <random>
namespace
{
std::random_device rd;
std::mt19937_64    gen(rd());

double buff[256*1024/sizeof(double)];
double cool_cache()
{
  return std::accumulate(std::begin(buff), std::end(buff), 0.0);
}
std::vector<int> populate_integers()
{
  std::vector<int> rv;
  std::generate_n(std::back_inserter(rv), 100000, [n=0]()mutable { return n++;});
  std::shuffle(std::begin(rv), std::end(rv), gen);
  return rv;
}
std::vector<std::string> populate_from_file(const char *filename)
{
  std::vector<std::string> rv;
  std::ifstream            in(filename);
  std::string              s;
  while (std::getline(in, s))
  {
    rv.push_back(s);
  }

  std::shuffle(std::begin(rv), std::end(rv), gen);
  return rv;
}
const std::vector<std::string> &paths()
{
  static auto rv = populate_from_file("full_paths");
  return rv;
}

const std::vector<std::string> &names()
{
  static auto rv = populate_from_file("individual_names");
  return rv;
}

const std::vector<int>& integers()
{
  static auto rv = populate_integers();
  return rv;
}

template <typename T>
auto consume(const T& t, const std::string& s)
{
  return &t + s.length();
}

}
template <typename Container, typename Src>
bool BM_populate(benchmark::State& state, Container c, const Src& src)
{
  auto e = src.end();
  auto b = src.begin();
  bool rv = false;
  while (state.KeepRunning())
  {
    state.PauseTiming();
    benchmark::DoNotOptimize(cool_cache());
    state.ResumeTiming();
    auto i = b;
    auto size = state.range(0);
    while (size--)
    {
      auto const inserted = c.insert(std::make_pair(*i, std::string())).second;
      benchmark::DoNotOptimize(rv = rv || inserted);
      if (++i == e) i = b;
    }
  }
  return rv;
}

template <typename Container, typename Src>
size_t BM_lookup_found(benchmark::State& state, Container c, const Src& src)
{
  const auto num_elems = state.range(0);
  for (size_t i = 0; i != num_elems; ++i)
  {
    c.insert(std::make_pair(src[i], std::string{}));
  }
  size_t rv = 0;
  while (state.KeepRunning())
  {
    state.PauseTiming();
    benchmark::DoNotOptimize(cool_cache());
    state.ResumeTiming();
    auto const max = state.range(0);
    for (size_t i = 0; i != max; ++i)
    {
      auto const found = c.count(src[i]);
      benchmark::DoNotOptimize(rv += found);
    }
  }
  return rv;
}

template <typename Container, typename Src>
size_t BM_lookup_fail(benchmark::State& state, Container c, const Src& src)
{
  const auto num_elems = state.range(0);
  for (size_t i = 0; i != num_elems; ++i)
  {
    c.insert(std::make_pair(src[i], std::string{}));
  }
  size_t rv = 0;
  while (state.KeepRunning())
  {
    state.PauseTiming();
    benchmark::DoNotOptimize(cool_cache());
    state.ResumeTiming();
    for (size_t i = 0; i != num_elems; ++i)
    {
      auto const found = c.count(src[num_elems + i]);
      benchmark::DoNotOptimize(rv += found);
    }
  }
  return rv;
}

template <typename Container, typename Src>
size_t BM_erase(benchmark::State& state, Container c, const Src& src)
{
  std::vector<typename Src::value_type> values(src.begin(), std::next(src.begin(), state.range(0)));
  for (auto& v : values)
  {
    c.insert(std::make_pair(v, std::string{}));
  }
  size_t rv = 0;
  while (state.KeepRunning())
  {
    state.PauseTiming();
    auto copy = c;
    std::shuffle(std::begin(values), std::end(values), gen);
    state.PauseTiming();
    benchmark::DoNotOptimize(cool_cache());
    state.ResumeTiming();
    state.ResumeTiming();
    for (auto& v : values)
    {
      auto n = copy.erase(v);
      benchmark::DoNotOptimize(rv += n);
    }
  }
  return rv;
}

template <typename Container, typename Src>
void BM_iterate(benchmark::State& state, Container c, const Src& src)
{
  for (size_t i = 0; i != state.range(0); ++i)
  {
    c.insert(std::make_pair(src[i], std::string()));
  }
  while (state.KeepRunning())
  {
    state.PauseTiming();
    benchmark::DoNotOptimize(cool_cache());
    state.ResumeTiming();
    for (auto&& elem : c)
    {
      benchmark::DoNotOptimize(consume(elem.first, elem.second));
    }
  }
}

BENCHMARK_CAPTURE(BM_iterate, int_std_map, std::map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, int_std_unordered_map, std::unordered_map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, int_unordered_flatmap, unordered_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, int_flatmap, flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, int_unordered_split_flatmap, unordered_split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, int_split_flatmap, split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_iterate, long_string_std_map, std::map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, long_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, long_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, long_string_flatmap, flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, long_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, long_string_split_flatmap, split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_iterate, short_string_std_map, std::map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, short_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, short_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, short_string_flatmap, flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, short_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_iterate, short_string_split_flatmap, split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);


BENCHMARK_CAPTURE(BM_erase, int_std_map, std::map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, int_std_unordered_map, std::unordered_map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, int_unordered_flatmap, unordered_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, int_flatmap, flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, int_unordered_split_flatmap, unordered_split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, int_split_flatmap, split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_erase, long_string_std_map, std::map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, long_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, long_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, long_string_flatmap, flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, long_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, long_string_split_flatmap, split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_erase, short_string_std_map, std::map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, short_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, short_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, short_string_flatmap, flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, short_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_erase, short_string_split_flatmap, split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);

///

BENCHMARK_CAPTURE(BM_lookup_found, int_std_map, std::map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, int_std_unordered_map, std::unordered_map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, int_unordered_flatmap, unordered_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, int_flatmap, flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, int_unordered_split_flatmap, unordered_split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, int_split_flatmap, split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_lookup_found, long_string_std_map, std::map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, long_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, long_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, long_string_flatmap, flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, long_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, long_string_split_flatmap, split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_lookup_found, short_string_std_map, std::map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, short_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, short_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, short_string_flatmap, flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, short_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_found, short_string_split_flatmap, split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_lookup_fail, int_std_map, std::map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, int_std_unordered_map, std::unordered_map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, int_unordered_flatmap, unordered_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, int_flatmap, flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, int_unordered_split_flatmap, unordered_split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, int_split_flatmap, split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_lookup_fail, long_string_std_map, std::map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, long_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, long_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, long_string_flatmap, flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, long_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, long_string_split_flatmap, split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_lookup_fail, short_string_std_map, std::map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, short_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, short_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, short_string_flatmap, flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, short_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_lookup_fail, short_string_split_flatmap, split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_populate, int_std_map, std::map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, int_std_unordered_map, std::unordered_map<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, int_unordered_flatmap, unordered_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, int_flatmap, flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, int_unordered_split_flatmap, unordered_split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, int_split_flatmap, split_flatmap<int, std::string>{}, integers())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_populate, long_string_std_map, std::map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, long_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, long_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, long_string_flatmap, flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, long_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, long_string_split_flatmap, split_flatmap<std::string, std::string>{}, paths())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_CAPTURE(BM_populate, short_string_std_map, std::map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, short_string_std_unordered_map, std::unordered_map<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, short_string_unordered_flatmap, unordered_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, short_string_flatmap, flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, short_string_unordered_split_flatmap, unordered_split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);
BENCHMARK_CAPTURE(BM_populate, short_string_split_flatmap, split_flatmap<std::string, std::string>{}, names())->RangeMultiplier(2)->Range(2, 2<<13);

BENCHMARK_MAIN();
