#include <jemalloc/jemalloc.h>
#include "util/bwtree_test_util.h"
#include "util/multithread_test_util.h"
#include "util/test_harness.h"

namespace terrier {

/**
 * These tests are adapted from https://github.com/wangziqi2013/BwTree/tree/master/test
 * Please do not use these as a model for other tests within this repository.
 */
struct BwTreeTests : public TerrierTest {
  void SetUp() override { TerrierTest::SetUp(); }

  void TearDown() override { TerrierTest::TearDown(); }

  const uint32_t num_threads_ = 4;
};

void PrintBasicMemoryStats() { malloc_stats_print(NULL, NULL, NULL); }

/**
 * Adapted from https://github.com/wangziqi2013/BwTree/blob/master/test/iterator_test.cpp
 */
// NOLINTNEXTLINE
TEST_F(BwTreeTests, MemoryUsage) {
  PrintBasicMemoryStats();
  auto *const tree = BwTreeTestUtil::GetEmptyTree();
  const uint32_t num_keys_ = 1000000 * 100;

  auto workload = [&](uint32_t id) {
    const uint32_t gcid = id + 1;
    tree->AssignGCID(gcid);

    uint32_t start_key = num_keys_ / num_threads_ * id;
    uint32_t end_key = start_key + num_keys_ / num_threads_ - 1;

    std::cout << start_key << " " << end_key << std::endl;

    for (uint32_t i = start_key; i < end_key; i++) {
      BigKey key;
      key.data_[0] = i;
      tree->Insert(key, i);
    }
    tree->UnregisterThread(gcid);
  };

  common::WorkerPool *thread_pool = new common::WorkerPool(num_threads_, {});

  MultiThreadTestUtil::RunThreadsUntilFinish(thread_pool, num_threads_, workload);

  delete thread_pool;

  PrintBasicMemoryStats();

  delete tree;
  PrintBasicMemoryStats();
}

}  // namespace terrier
