#undef NDEBUG
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <vector>
#include "benchmark/benchmark.h"
#include "output_test.h"

#if !defined(BENCHMARK_NO_CXX11)
using std::log2;
#elif defined(_MSC_VER) && _MSC_VER < 1900
static double log2(double x) { return log(x) / log(2.0); }
#endif

namespace {

#define ADD_COMPLEXITY_CASES(...) \
  int CONCAT(dummy, __LINE__) = AddComplexityTest(__VA_ARGS__)

int AddComplexityTest(std::string big_o_test_name, std::string rms_test_name,
                      std::string big_o) {
  SET_SUBSTITUTIONS({{"%bigo_name", big_o_test_name},
                     {"%rms_name", rms_test_name},
                     {"%bigo_str", "[ ]* %float " + big_o},
                     {"%bigo", big_o},
                     {"%rms", "[ ]*[0-9]+ %"}});
  ADD_CASES(TC_ConsoleOut,
            {TestCase("^%bigo_name %bigo_str %bigo_str[ ]*$"),
             TestCase("^%bigo_name",
                      MR_Not),  // Assert we we didn't only matched a name.
             TestCase("^%rms_name %rms %rms[ ]*$", MR_Next)});
  ADD_CASES(TC_JSONOut,
            {TestCase("\"name\": \"%bigo_name\",$"),
             TestCase("\"cpu_coefficient\": [0-9]+,$", MR_Next),
             TestCase("\"real_coefficient\": [0-9]{1,5},$", MR_Next),
             TestCase("\"big_o\": \"%bigo\",$", MR_Next),
             TestCase("\"time_unit\": \"ns\"$", MR_Next),
             TestCase("}", MR_Next), TestCase("\"name\": \"%rms_name\",$"),
             TestCase("\"rms\": %float$", MR_Next), TestCase("}", MR_Next)});
  ADD_CASES(TC_CSVOut,
            {TestCase("^\"%bigo_name\",,%float,%float,%bigo,,,,,$"),
             TestCase("^\"%bigo_name\"", MR_Not),
             TestCase("^\"%rms_name\",,%float,%float,,,,,,$", MR_Next)});
  return 0;
}

}  // end namespace

// ========================================================================= //
// --------------------------- Testing BigO O(1) --------------------------- //
// ========================================================================= //

void BM_Complexity_O1(benchmark::State& state) {
  while (state.KeepRunning()) {
    for (int i = 0; i < 1024; ++i) {
      benchmark::DoNotOptimize(&i);
    }
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Complexity_O1)->Range(1, 1 << 18)->Complexity(benchmark::o1);
BENCHMARK(BM_Complexity_O1)->Range(1, 1 << 18)->Complexity();
static double o1(int) { return 1.0; }
BENCHMARK(BM_Complexity_O1)->Range(1, 1 << 18)->Complexity(o1);

const char* big_o_1_test_name = "BM_Complexity_O1_BigO";
const char* rms_o_1_test_name = "BM_Complexity_O1_RMS";
const char* enum_big_o_1 = "\\([0-9]+\\)";
// FIXME: Tolerate both '(1)' and 'lgN' as output when the complexity is auto
// deduced.
// See https://github.com/google/benchmark/issues/272
const char* auto_big_o_1 = "(\\([0-9]+\\))|(lgN)";
const char* lambda_big_o_1 = "f\\(N\\)";

// Add enum tests
ADD_COMPLEXITY_CASES(big_o_1_test_name, rms_o_1_test_name, enum_big_o_1);

// Add auto enum tests
ADD_COMPLEXITY_CASES(big_o_1_test_name, rms_o_1_test_name, auto_big_o_1);

// Add lambda tests
ADD_COMPLEXITY_CASES(big_o_1_test_name, rms_o_1_test_name, lambda_big_o_1);

// ========================================================================= //
// --------------------------- Testing BigO O(N) --------------------------- //
// ========================================================================= //

std::vector<int> ConstructRandomVector(int size) {
  std::vector<int> v;
  v.reserve(size);
  for (int i = 0; i < size; ++i) {
    v.push_back(std::rand() % size);
  }
  return v;
}

void BM_Complexity_O_N(benchmark::State& state) {
  std::vector<int> v = ConstructRandomVector(state.range(0));
  const int item_not_in_vector =
      state.range(0) * 2;  // Test worst case scenario (item not in vector)
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(std::find(v.begin(), v.end(), item_not_in_vector));
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Complexity_O_N)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Complexity(benchmark::oN);
static double oN(int n) { return n; }
BENCHMARK(BM_Complexity_O_N)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Complexity(oN);
BENCHMARK(BM_Complexity_O_N)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Complexity();

const char* big_o_n_test_name = "BM_Complexity_O_N_BigO";
const char* rms_o_n_test_name = "BM_Complexity_O_N_RMS";
const char* enum_auto_big_o_n = "N";
const char* lambda_big_o_n = "f\\(N\\)";

// Add enum tests
ADD_COMPLEXITY_CASES(big_o_n_test_name, rms_o_n_test_name, enum_auto_big_o_n);

// Add lambda tests
ADD_COMPLEXITY_CASES(big_o_n_test_name, rms_o_n_test_name, lambda_big_o_n);

// ========================================================================= //
// ------------------------- Testing BigO O(N*lgN) ------------------------- //
// ========================================================================= //

static void BM_Complexity_O_N_log_N(benchmark::State& state) {
  std::vector<int> v = ConstructRandomVector(state.range(0));
  while (state.KeepRunning()) {
    std::sort(v.begin(), v.end());
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Complexity_O_N_log_N)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Complexity(benchmark::oNLogN);
static double oNLogN(int n) { return n * log2(n); }
BENCHMARK(BM_Complexity_O_N_log_N)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Complexity(oNLogN);
BENCHMARK(BM_Complexity_O_N_log_N)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Complexity();

const char* big_o_n_lg_n_test_name = "BM_Complexity_O_N_log_N_BigO";
const char* rms_o_n_lg_n_test_name = "BM_Complexity_O_N_log_N_RMS";
const char* enum_auto_big_o_n_lg_n = "NlgN";
const char* lambda_big_o_n_lg_n = "f\\(N\\)";

// Add enum tests
ADD_COMPLEXITY_CASES(big_o_n_lg_n_test_name, rms_o_n_lg_n_test_name,
                     enum_auto_big_o_n_lg_n);

// Add lambda tests
ADD_COMPLEXITY_CASES(big_o_n_lg_n_test_name, rms_o_n_lg_n_test_name,
                     lambda_big_o_n_lg_n);

// ========================================================================= //
// --------------------------- TEST CASES END ------------------------------ //
// ========================================================================= //

int main(int argc, char* argv[]) { RunOutputTests(argc, argv); }
