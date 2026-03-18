#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "sparse-set.hpp"

struct TestId {
  uint64_t m_id;

  TestId() : m_id(0) {}
  explicit TestId(uint64_t id) : m_id(id) {}

  operator uint64_t() const { return m_id; }
};

class SparseSetTest : public ::testing::Test {
protected:
  SparseSet<TestId> m_set;
};

TEST_F(SparseSetTest, EmptySetHasSizeZero) { EXPECT_EQ(m_set.size(), 0); }

TEST_F(SparseSetTest, InsertSingleElement) {
  m_set.insert(TestId(1));
  EXPECT_EQ(m_set.size(), 1);
  EXPECT_TRUE(m_set.contains(1));
}

TEST_F(SparseSetTest, InsertMultipleElements) {
  m_set.insert(TestId(1));
  m_set.insert(TestId(100));
  m_set.insert(TestId(1000));
  EXPECT_EQ(m_set.size(), 3);
  EXPECT_TRUE(m_set.contains(1));
  EXPECT_TRUE(m_set.contains(100));
  EXPECT_TRUE(m_set.contains(1000));
}

TEST_F(SparseSetTest, ContainsReturnsFalseForNonExistent) {
  m_set.insert(TestId(5));
  EXPECT_FALSE(m_set.contains(1));
  EXPECT_FALSE(m_set.contains(100));
}

TEST_F(SparseSetTest, ContainsReturnsFalseForRemovedElement) {
  m_set.insert(TestId(10));
  EXPECT_TRUE(m_set.contains(10));
  m_set.remove(10);
  EXPECT_FALSE(m_set.contains(10));
}

TEST_F(SparseSetTest, RemoveNonExistentReturnsFalse) {
  EXPECT_FALSE(m_set.remove(1));
}

TEST_F(SparseSetTest, RemoveExistingElement) {
  m_set.insert(TestId(50));
  EXPECT_TRUE(m_set.contains(50));
  EXPECT_TRUE(m_set.remove(50));
  EXPECT_FALSE(m_set.contains(50));
}

TEST_F(SparseSetTest, LargeIdInsertion) {
  uint64_t large_id = 100000;
  m_set.insert(TestId(large_id));
  EXPECT_EQ(m_set.size(), 1);
  EXPECT_TRUE(m_set.contains(large_id));
}

TEST_F(SparseSetTest, MultipleInsertsAndRemoves) {
  for (uint64_t i = 0; i < 100; ++i) {
    m_set.insert(TestId(i));
  }
  EXPECT_EQ(m_set.size(), 100);

  for (uint64_t i = 0; i < 100; i += 2) {
    m_set.remove(i);
  }

  for (uint64_t i = 0; i < 100; ++i) {
    if (i % 2 == 0) {
      EXPECT_FALSE(m_set.contains(i));
    } else {
      EXPECT_TRUE(m_set.contains(i));
    }
  }
}

TEST_F(SparseSetTest, BucketBoundaryTest) {
  m_set.insert(TestId(63));
  m_set.insert(TestId(64));
  m_set.insert(TestId(127));
  m_set.insert(TestId(128));

  EXPECT_TRUE(m_set.contains(63));
  EXPECT_TRUE(m_set.contains(64));
  EXPECT_TRUE(m_set.contains(127));
  EXPECT_TRUE(m_set.contains(128));
}

TEST(SparseSetConcurrentTest, ConcurrentInsert) {
  SparseSet<TestId> set;
  std::vector<std::thread> threads;
  constexpr size_t num_threads = 4;
  constexpr size_t elements_per_thread = 1000;

  for (size_t t = 0; t < num_threads; ++t) {
    threads.emplace_back([&set, t]() {
      for (size_t i = 0; i < elements_per_thread; ++i) {
        uint64_t id = t * elements_per_thread + i;
        set.insert(TestId(id));
      }
    });
  }

  for (auto &th : threads) {
    th.join();
  }

  EXPECT_EQ(set.size(), num_threads * elements_per_thread);
}

TEST(SparseSetConcurrentTest, ConcurrentInsertContains) {
  SparseSet<TestId> set;
  std::vector<std::thread> threads;
  constexpr size_t num_threads = 4;
  constexpr size_t elements_per_thread = 500;

  for (size_t t = 0; t < num_threads; ++t) {
    threads.emplace_back([&set, t]() {
      for (size_t i = 0; i < elements_per_thread; ++i) {
        uint64_t id = t * elements_per_thread + i;
        set.insert(TestId(id));
      }
    });
  }

  for (auto &th : threads) {
    th.join();
  }

  EXPECT_EQ(set.size(), num_threads * elements_per_thread);
}