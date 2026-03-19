#ifndef ARCHETYPE_HPP
#define ARCHETYPE_HPP

#include <cstddef>
#include <memory>
#include <vector>

#include "archetype-chunk.hpp"
#include "entity.hpp"

constexpr size_t L1_CACHE_SIZE = 32 * 1024;
constexpr size_t DEFAULT_CHUNK_CAPACITY = 64;

template <typename... Components> class Archetype {
public:
  static constexpr size_t chunk_capacity = DEFAULT_CHUNK_CAPACITY;

  Archetype() : m_entity_count(0), m_next_entity_id(1) {
    m_chunks.push_back(std::make_unique<ArchetypeChunk<Components...>>());
  }

  ~Archetype() = default;
  Archetype(const Archetype &) = delete;
  Archetype &operator=(const Archetype &) = delete;
  Archetype(Archetype &&) = default;
  Archetype &operator=(Archetype &&) = default;

  Entity create_entity();
  bool destroy_entity(Entity entity);
  bool contains(Entity entity) const;

  template <typename Component> Component *get_component(Entity entity);

  template <typename Component>
  const Component *get_component(Entity entity) const;

  template <typename Component>
  void set_component(Entity entity, const Component &component);

  template <typename Func> void for_each_entity(Func &&func);

  template <typename Func> void for_each_entity(Func &&func) const;

  template <typename Func> void for_each(Func &&func);

  template <typename Func> void for_each(Func &&func) const;

  size_t entity_count() const {
    return m_entity_count.load(std::memory_order_acquire);
  }
  size_t chunk_count() const { return m_chunks.size(); }

private:
  size_t calculate_chunk_index(Entity entity) const;
  ArchetypeChunk<Components...> *get_or_create_chunk(size_t index);

  std::vector<std::unique_ptr<ArchetypeChunk<Components...>>> m_chunks;
  std::vector<size_t> m_entity_to_chunk;
  std::atomic<size_t> m_entity_count;
  std::atomic<size_t> m_next_entity_id;
};

template <typename... Components>
size_t Archetype<Components...>::calculate_chunk_index(Entity entity) const {
  return (entity.m_id - 1) / CHUNK_CAPACITY;
}

template <typename... Components>
ArchetypeChunk<Components...> *
Archetype<Components...>::get_or_create_chunk(size_t index) {
  if (index >= m_chunks.size()) {
    size_t old_size = m_chunks.size();
    size_t new_size = index + 1;
    m_chunks.resize(new_size);
    for (size_t i = old_size; i < new_size; ++i) {
      m_chunks[i] = std::make_unique<ArchetypeChunk<Components...>>();
    }
  }
  return m_chunks[index].get();
}

template <typename... Components>
Entity Archetype<Components...>::create_entity() {
  size_t entity_id = m_next_entity_id.fetch_add(1, std::memory_order_acq_rel);
  Entity entity(entity_id + 1);

  size_t chunk_idx = calculate_chunk_index(entity);
  ArchetypeChunk<Components...> *chunk = get_or_create_chunk(chunk_idx);

  if (chunk->size() >= CHUNK_CAPACITY) {
    chunk_idx = m_chunks.size();
    chunk = get_or_create_chunk(chunk_idx);
  }

  chunk->add_entity(entity);

  if (m_entity_to_chunk.size() <= entity_id) {
    m_entity_to_chunk.resize(entity_id + 1);
  }
  m_entity_to_chunk[entity_id] = chunk_idx;

  m_entity_count.fetch_add(1, std::memory_order_acq_rel);

  return entity;
}

template <typename... Components>
bool Archetype<Components...>::destroy_entity(Entity entity) {
  if (entity.m_id == 0 ||
      entity.m_id > m_next_entity_id.load(std::memory_order_acquire)) {
    return false;
  }

  size_t entity_idx = entity.m_id - 1;
  if (entity_idx >= m_entity_to_chunk.size()) {
    return false;
  }

  size_t chunk_idx = m_entity_to_chunk[entity_idx];
  if (chunk_idx >= m_chunks.size()) {
    return false;
  }

  ArchetypeChunk<Components...> *chunk = m_chunks[chunk_idx].get();
  if (!chunk->contains(entity)) {
    return false;
  }

  chunk->remove_entity(entity);
  m_entity_to_chunk[entity_idx] = static_cast<size_t>(-1);
  m_entity_count.fetch_sub(1, std::memory_order_acq_rel);

  return true;
}

template <typename... Components>
bool Archetype<Components...>::contains(Entity entity) const {
  if (entity.m_id == 0 ||
      entity.m_id > m_next_entity_id.load(std::memory_order_acquire)) {
    return false;
  }

  size_t entity_idx = entity.m_id - 1;
  if (entity_idx >= m_entity_to_chunk.size()) {
    return false;
  }

  size_t chunk_idx = m_entity_to_chunk[entity_idx];
  if (chunk_idx == static_cast<size_t>(-1) || chunk_idx >= m_chunks.size()) {
    return false;
  }

  return m_chunks[chunk_idx]->contains(entity);
}

template <typename... Components>
template <typename Component>
Component *Archetype<Components...>::get_component(Entity entity) {
  if (entity.m_id == 0 ||
      entity.m_id > m_next_entity_id.load(std::memory_order_acquire)) {
    return nullptr;
  }

  size_t entity_idx = entity.m_id - 1;
  if (entity_idx >= m_entity_to_chunk.size()) {
    return nullptr;
  }

  size_t chunk_idx = m_entity_to_chunk[entity_idx];
  if (chunk_idx == static_cast<size_t>(-1) || chunk_idx >= m_chunks.size()) {
    return nullptr;
  }

  return m_chunks[chunk_idx]->template get_component<Component>(entity);
}

template <typename... Components>
template <typename Component>
const Component *Archetype<Components...>::get_component(Entity entity) const {
  if (entity.m_id == 0 ||
      entity.m_id > m_next_entity_id.load(std::memory_order_acquire)) {
    return nullptr;
  }

  size_t entity_idx = entity.m_id - 1;
  if (entity_idx >= m_entity_to_chunk.size()) {
    return nullptr;
  }

  size_t chunk_idx = m_entity_to_chunk[entity_idx];
  if (chunk_idx == static_cast<size_t>(-1) || chunk_idx >= m_chunks.size()) {
    return nullptr;
  }

  return m_chunks[chunk_idx]->template get_component<Component>(entity);
}

template <typename... Components>
template <typename Component>
void Archetype<Components...>::set_component(Entity entity,
                                             const Component &component) {
  if (entity.m_id == 0 ||
      entity.m_id > m_next_entity_id.load(std::memory_order_acquire)) {
    return;
  }

  size_t entity_idx = entity.m_id - 1;
  if (entity_idx >= m_entity_to_chunk.size()) {
    return;
  }

  size_t chunk_idx = m_entity_to_chunk[entity_idx];
  if (chunk_idx == static_cast<size_t>(-1) || chunk_idx >= m_chunks.size()) {
    return;
  }

  m_chunks[chunk_idx]->template set_component<Component>(entity, component);
}

template <typename... Components>
template <typename Func>
void Archetype<Components...>::for_each_entity(Func &&func) {
  for (auto &chunk : m_chunks) {
    if (!chunk->empty()) {
      chunk->for_each_entity(std::forward<Func>(func));
    }
  }
}

template <typename... Components>
template <typename Func>
void Archetype<Components...>::for_each_entity(Func &&func) const {
  for (auto &chunk : m_chunks) {
    if (!chunk->empty()) {
      chunk->for_each_entity(std::forward<Func>(func));
    }
  }
}

template <typename... Components>
template <typename Func>
void Archetype<Components...>::for_each(Func &&func) {
  for (auto &chunk : m_chunks) {
    if (!chunk->empty()) {
      chunk->for_each(std::forward<Func>(func));
    }
  }
}

template <typename... Components>
template <typename Func>
void Archetype<Components...>::for_each(Func &&func) const {
  for (auto &chunk : m_chunks) {
    if (!chunk->empty()) {
      chunk->for_each(std::forward<Func>(func));
    }
  }
}

#endif
