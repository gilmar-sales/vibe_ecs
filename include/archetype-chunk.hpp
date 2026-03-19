#ifndef ARCHETYPE_CHUNK_HPP
#define ARCHETYPE_CHUNK_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

#include "entity.hpp"
#include "sparse-set.hpp"

constexpr size_t CHUNK_CAPACITY = 64;

template <typename T>
concept Component = requires(T &t) {
  { t.m_id } -> std::convertible_to<uint64_t>;
};

template <typename T> struct ComponentData {
  std::vector<T> m_data;

  ComponentData() { m_data.reserve(CHUNK_CAPACITY); }
};

template <typename... Components> class ArchetypeChunk {
public:
  static constexpr size_t capacity = CHUNK_CAPACITY;
  static constexpr size_t num_components = sizeof...(Components);

  ArchetypeChunk();

  bool empty() const { return m_entities.size() == 0; }
  size_t size() const { return m_entities.size(); }
  size_t count() const { return m_entities.size(); }

  bool contains(Entity entity) const {
    return m_entities.contains(entity.m_id);
  }

  Entity add_entity(Entity entity);

  bool remove_entity(Entity entity);

  template <typename Component> Component *get_component(Entity entity);

  template <typename Component>
  const Component *get_component(Entity entity) const;

  template <typename Component>
  void set_component(Entity entity, const Component &component);

  template <typename Func> void for_each_entity(Func &&func);

  template <typename Func> void for_each_entity(Func &&func) const;

  auto begin() const { return m_entities.begin(); }
  auto end() const { return m_entities.end(); }

private:
  template <size_t Index> struct ComponentTypeAt {
    using type =
        typename std::tuple_element<Index, std::tuple<Components...>>::type;
  };

  template <size_t Index>
  ComponentData<typename ComponentTypeAt<Index>::type> &get_component_data() {
    return std::get<Index>(m_components);
  }

  template <size_t Index>
  const ComponentData<typename ComponentTypeAt<Index>::type> &
  get_component_data() const {
    return std::get<Index>(m_components);
  }

  size_t get_component_index(Entity entity) const;

  SparseSet<Entity> m_entities;
  std::tuple<ComponentData<Components>...> m_components;
  std::atomic<size_t> m_entity_index_counter;
};

namespace detail {
template <typename F, size_t... Is>
constexpr void for_each_index(F &&f, std::index_sequence<Is...>) {
  ((f(std::integral_constant<size_t, Is>{}), ...));
}
} // namespace detail

template <typename... Components>
ArchetypeChunk<Components...>::ArchetypeChunk() : m_entity_index_counter(0) {}

template <typename... Components>
Entity ArchetypeChunk<Components...>::add_entity(Entity entity) {
  if (m_entities.size() >= CHUNK_CAPACITY) {
    return Entity(0);
  }

  size_t entity_idx =
      m_entity_index_counter.fetch_add(1, std::memory_order_acq_rel);

  m_entities.insert(entity);

  detail::for_each_index(
      [this](auto Index) {
        using CompType =
            std::tuple_element_t<Index.value, std::tuple<Components...>>;
        std::get<Index.value>(m_components).m_data.emplace_back();
      },
      std::make_index_sequence<sizeof...(Components)>{});

  return entity;
}

template <typename... Components>
bool ArchetypeChunk<Components...>::remove_entity(Entity entity) {
  return m_entities.remove(entity.m_id);
}

template <typename... Components>
template <typename Component>
Component *ArchetypeChunk<Components...>::get_component(Entity entity) {
  auto idx = get_component_index(entity);
  if (idx == static_cast<size_t>(-1)) {
    return nullptr;
  }

  using CompData = ComponentData<Component>;
  return &std::get<CompData>(m_components).m_data[idx];
}

template <typename... Components>
template <typename Component>
const Component *
ArchetypeChunk<Components...>::get_component(Entity entity) const {
  auto idx = get_component_index(entity);
  if (idx == static_cast<size_t>(-1)) {
    return nullptr;
  }

  using CompData = ComponentData<Component>;
  return &std::get<CompData>(m_components).m_data[idx];
}

template <typename... Components>
template <typename Component>
void ArchetypeChunk<Components...>::set_component(Entity entity,
                                                  const Component &component) {
  auto idx = get_component_index(entity);
  if (idx == static_cast<size_t>(-1)) {
    return;
  }

  using CompData = ComponentData<Component>;
  std::get<CompData>(m_components).m_data[idx] = component;
}

template <typename... Components>
template <typename Func>
void ArchetypeChunk<Components...>::for_each_entity(Func &&func) {
  for (const auto &entity : m_entities) {
    func(entity);
  }
}

template <typename... Components>
template <typename Func>
void ArchetypeChunk<Components...>::for_each_entity(Func &&func) const {
  for (const auto &entity : m_entities) {
    func(entity);
  }
}

template <typename... Components>
size_t ArchetypeChunk<Components...>::get_component_index(Entity entity) const {
  size_t sparse_bucket_idx = entity.m_id / 64;
  size_t sparse_offset = entity.m_id % 64;

  if (sparse_bucket_idx >= m_entities.bucket_count()) {
    return static_cast<size_t>(-1);
  }

  auto *bucket = m_entities.get_bucket(sparse_bucket_idx);
  if (bucket == nullptr) {
    return static_cast<size_t>(-1);
  }

  uint64_t dense_position = bucket->get(sparse_offset);
  if (dense_position == UINT64_MAX) {
    return static_cast<size_t>(-1);
  }

  return static_cast<size_t>(dense_position);
}

#endif
