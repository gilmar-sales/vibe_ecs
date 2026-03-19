#ifndef SPARSE_SET_HPP
#define SPARSE_SET_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

template <typename T>
concept ConvertibleToUint64 = requires(T value) {
  { static_cast<uint64_t>(value) };
};

constexpr size_t BUCKET_SIZE = 64;
constexpr uint64_t INVALID_INDEX = std::numeric_limits<uint64_t>::max();

template <typename T> struct SparseBucket {
  std::atomic<uint64_t> m_indices[BUCKET_SIZE];
  std::atomic<SparseBucket *> m_next;

  SparseBucket() : m_next(nullptr) {
    for (auto &idx : m_indices) {
      idx.store(INVALID_INDEX, std::memory_order_relaxed);
    }
  }

  uint64_t get(size_t index) const {
    return m_indices[index].load(std::memory_order_acquire);
  }

  bool compare_exchange(size_t index, uint64_t &expected, uint64_t desired) {
    return m_indices[index].compare_exchange_strong(expected, desired,
                                                    std::memory_order_acq_rel,
                                                    std::memory_order_acquire);
  }

  void store(size_t index, uint64_t value) {
    m_indices[index].store(value, std::memory_order_release);
  }

  SparseBucket *next() const { return m_next.load(std::memory_order_acquire); }

  void set_next(SparseBucket *next_bucket) {
    m_next.store(next_bucket, std::memory_order_release);
  }
};

template <typename T> class SparseSet {
  static_assert(
      ConvertibleToUint64<T>,
      "Type T must be convertible to uint64_t (implement operator uint64_t())");

public:
  SparseSet();
  ~SparseSet();

  void insert(T value);
  bool contains(uint64_t id) const;
  bool remove(uint64_t id);
  size_t size() const;
  size_t active_count() const;
  inline void for_each(auto &&callback) const;
  inline void for_each_with_index(auto &&callback) const;
  size_t bucket_count() const;
  SparseBucket<T> *get_bucket(size_t idx) const;

  auto begin() const;
  auto end() const;

private:
  size_t calculate_bucket_index(uint64_t id) const;
  SparseBucket<T> *get_or_create_sparse_bucket(size_t bucket_idx);

  std::vector<std::atomic<SparseBucket<T> *> *> m_sparse_buckets;
  std::vector<T> m_dense_data;
  std::atomic<size_t> m_count;
};

template <typename T> SparseSet<T>::SparseSet() : m_count(0) {
  m_sparse_buckets.resize(1024);
  for (size_t i = 0; i < 1024; ++i) {
    m_sparse_buckets[i] = new std::atomic<SparseBucket<T> *>(nullptr);
  }
  m_dense_data.reserve(1024 * BUCKET_SIZE);
}

template <typename T> SparseSet<T>::~SparseSet() {
  for (auto &bucket_ptr : m_sparse_buckets) {
    SparseBucket<T> *bucket = bucket_ptr->load(std::memory_order_acquire);
    while (bucket) {
      SparseBucket<T> *next = bucket->next();
      delete bucket;
      bucket = next;
    }
    delete bucket_ptr;
  }
}

template <typename T>
size_t SparseSet<T>::calculate_bucket_index(uint64_t id) const {
  return id / BUCKET_SIZE;
}

template <typename T>
SparseBucket<T> *SparseSet<T>::get_or_create_sparse_bucket(size_t bucket_idx) {
  if (bucket_idx >= m_sparse_buckets.size()) {
    size_t old_size = m_sparse_buckets.size();
    size_t new_size = bucket_idx + 1;
    m_sparse_buckets.resize(new_size);
    for (size_t i = old_size; i < new_size; ++i) {
      m_sparse_buckets[i] = new std::atomic<SparseBucket<T> *>(nullptr);
    }
  }

  SparseBucket<T> *expected = nullptr;
  SparseBucket<T> *desired = new SparseBucket<T>();

  SparseBucket<T> *result =
      m_sparse_buckets[bucket_idx]->load(std::memory_order_acquire);
  if (result != nullptr) {
    delete desired;
    return result;
  }

  bool success = m_sparse_buckets[bucket_idx]->compare_exchange_strong(
      expected, desired, std::memory_order_acq_rel, std::memory_order_acquire);

  if (!success) {
    delete desired;
    return expected;
  }

  return desired;
}

template <typename T> void SparseSet<T>::insert(T value) {
  uint64_t id = static_cast<uint64_t>(value);

  size_t sparse_bucket_idx = calculate_bucket_index(id);
  size_t sparse_offset = id % BUCKET_SIZE;

  SparseBucket<T> *sparse_bucket =
      get_or_create_sparse_bucket(sparse_bucket_idx);

  uint64_t dense_position = m_count.fetch_add(1, std::memory_order_acq_rel);

  m_dense_data.push_back(value);

  sparse_bucket->store(sparse_offset, dense_position);
}

template <typename T> bool SparseSet<T>::contains(uint64_t id) const {
  size_t sparse_bucket_idx = calculate_bucket_index(id);

  if (sparse_bucket_idx >= m_sparse_buckets.size()) {
    return false;
  }

  SparseBucket<T> *sparse_bucket =
      m_sparse_buckets[sparse_bucket_idx]->load(std::memory_order_acquire);
  if (sparse_bucket == nullptr) {
    return false;
  }

  size_t sparse_offset = id % BUCKET_SIZE;
  uint64_t dense_position = sparse_bucket->get(sparse_offset);

  if (dense_position == INVALID_INDEX) {
    return false;
  }

  if (dense_position >= m_dense_data.size()) {
    return false;
  }

  uint64_t stored_id = static_cast<uint64_t>(m_dense_data[dense_position]);
  return stored_id == id;
}

template <typename T> bool SparseSet<T>::remove(uint64_t id) {
  size_t sparse_bucket_idx = calculate_bucket_index(id);

  if (sparse_bucket_idx >= m_sparse_buckets.size()) {
    return false;
  }

  SparseBucket<T> *sparse_bucket =
      m_sparse_buckets[sparse_bucket_idx]->load(std::memory_order_acquire);
  if (sparse_bucket == nullptr) {
    return false;
  }

  size_t sparse_offset = id % BUCKET_SIZE;

  uint64_t current_value = sparse_bucket->get(sparse_offset);
  if (current_value == INVALID_INDEX) {
    return false;
  }

  uint64_t expected = current_value;
  uint64_t desired = INVALID_INDEX;

  bool success =
      sparse_bucket->compare_exchange(sparse_offset, expected, desired);

  if (!success) {
    return false;
  }

  return true;
}

template <typename T> size_t SparseSet<T>::size() const {
  return m_count.load(std::memory_order_acquire);
}

template <typename T> size_t SparseSet<T>::active_count() const {
  size_t count = 0;
  size_t num_buckets = m_sparse_buckets.size();

  for (size_t bucket_idx = 0; bucket_idx < num_buckets; ++bucket_idx) {
    SparseBucket<T> *bucket =
        m_sparse_buckets[bucket_idx]->load(std::memory_order_acquire);
    while (bucket) {
      for (size_t i = 0; i < BUCKET_SIZE; ++i) {
        uint64_t pos = bucket->get(i);
        if (pos != INVALID_INDEX) {
          ++count;
        }
      }
      bucket = bucket->next();
    }
  }

  return count;
}

template <typename T> void SparseSet<T>::for_each(auto &&callback) const {
  for (auto &item : m_dense_data) {
    callback(item);
  }
}

template <typename T>
void SparseSet<T>::for_each_with_index(auto &&callback) const {
  for (auto index = 0; index < m_dense_data.size(); ++index) {
    callback(index, m_dense_data[index]);
  }
}

template <typename T> size_t SparseSet<T>::bucket_count() const {
  return m_sparse_buckets.size();
}

template <typename T>
SparseBucket<T> *SparseSet<T>::get_bucket(size_t idx) const {
  if (idx >= m_sparse_buckets.size()) {
    return nullptr;
  }
  return m_sparse_buckets[idx]->load(std::memory_order_acquire);
}

template <typename T> auto SparseSet<T>::begin() const {
  return m_dense_data.begin();
}

template <typename T> auto SparseSet<T>::end() const {
  return m_dense_data.end();
}

#endif
