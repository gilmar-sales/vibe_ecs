#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <cstdint>

struct Entity {
  uint64_t m_id;

  Entity() : m_id(0) {}
  explicit Entity(uint64_t id) : m_id(id) {}

  operator uint64_t() const { return m_id; }

  bool operator==(const Entity& other) const { return m_id == other.m_id; }
  bool operator!=(const Entity& other) const { return m_id != other.m_id; }
};

#endif