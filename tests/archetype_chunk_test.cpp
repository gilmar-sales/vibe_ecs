#include <gtest/gtest.h>

#include "entity.hpp"
#include "archetype-chunk.hpp"

struct TestPositionComponent {
  uint64_t m_id;
  float m_x, m_y;

  TestPositionComponent() : m_id(0), m_x(0), m_y(0) {}
  TestPositionComponent(uint64_t id, float x, float y) : m_id(id), m_x(x), m_y(y) {}

  operator uint64_t() const { return m_id; }
};

struct TestVelocityComponent {
  uint64_t m_id;
  float m_vx, m_vy;

  TestVelocityComponent() : m_id(0), m_vx(0), m_vy(0) {}
  TestVelocityComponent(uint64_t id, float vx, float vy) : m_id(id), m_vx(vx), m_vy(vy) {}

  operator uint64_t() const { return m_id; }
};

using TestChunk = ArchetypeChunk<TestPositionComponent, TestVelocityComponent>;

class ArchetypeChunkTest : public ::testing::Test {
protected:
  TestChunk m_chunk;
};

TEST_F(ArchetypeChunkTest, EmptyChunkHasSizeZero) {
  EXPECT_EQ(m_chunk.size(), 0);
}

TEST_F(ArchetypeChunkTest, AddSingleEntity) {
  Entity e(1);
  m_chunk.add_entity(e);

  EXPECT_EQ(m_chunk.size(), 1);
  EXPECT_TRUE(m_chunk.contains(e));
}

TEST_F(ArchetypeChunkTest, AddMultipleEntities) {
  for (uint64_t i = 1; i <= 10; ++i) {
    Entity e(i);
    m_chunk.add_entity(e);
  }

  EXPECT_EQ(m_chunk.size(), 10);

  for (uint64_t i = 1; i <= 10; ++i) {
    EXPECT_TRUE(m_chunk.contains(Entity(i)));
  }
}

TEST_F(ArchetypeChunkTest, SetAndGetComponent) {
  Entity e(1);
  m_chunk.add_entity(e);

  TestPositionComponent pos(1, 10.5f, 20.5f);
  m_chunk.set_component(e, pos);

  auto* retrieved = m_chunk.get_component<TestPositionComponent>(e);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_FLOAT_EQ(retrieved->m_x, 10.5f);
  EXPECT_FLOAT_EQ(retrieved->m_y, 20.5f);
}

TEST_F(ArchetypeChunkTest, SetMultipleComponents) {
  Entity e(1);
  m_chunk.add_entity(e);

  TestPositionComponent pos(1, 10.0f, 20.0f);
  m_chunk.set_component(e, pos);

  TestVelocityComponent vel(1, 5.0f, -3.0f);
  m_chunk.set_component(e, vel);

  auto* retrieved_pos = m_chunk.get_component<TestPositionComponent>(e);
  auto* retrieved_vel = m_chunk.get_component<TestVelocityComponent>(e);

  ASSERT_NE(retrieved_pos, nullptr);
  ASSERT_NE(retrieved_vel, nullptr);

  EXPECT_FLOAT_EQ(retrieved_pos->m_x, 10.0f);
  EXPECT_FLOAT_EQ(retrieved_pos->m_y, 20.0f);
  EXPECT_FLOAT_EQ(retrieved_vel->m_vx, 5.0f);
  EXPECT_FLOAT_EQ(retrieved_vel->m_vy, -3.0f);
}

TEST_F(ArchetypeChunkTest, RemoveEntity) {
  Entity e(1);
  m_chunk.add_entity(e);

  EXPECT_TRUE(m_chunk.contains(e));
  EXPECT_TRUE(m_chunk.remove_entity(e));
  EXPECT_FALSE(m_chunk.contains(e));
}

TEST_F(ArchetypeChunkTest, RemoveNonExistentReturnsFalse) {
  Entity e(1);
  EXPECT_FALSE(m_chunk.remove_entity(e));
}

TEST_F(ArchetypeChunkTest, GetComponentForNonExistentReturnsNull) {
  Entity e(1);
  auto* component = m_chunk.get_component<TestPositionComponent>(e);
  EXPECT_EQ(component, nullptr);
}

TEST_F(ArchetypeChunkTest, ForEachEntity) {
  for (uint64_t i = 1; i <= 5; ++i) {
    Entity e(i);
    m_chunk.add_entity(e);
  }

  int count = 0;
  m_chunk.for_each_entity([&count](Entity e) { count++; });

  EXPECT_EQ(count, 5);
}

TEST_F(ArchetypeChunkTest, RangeBasedFor) {
  for (uint64_t i = 1; i <= 5; ++i) {
    Entity e(i);
    m_chunk.add_entity(e);
  }

  int count = 0;
  for (Entity e : m_chunk) {
    (void)e;
    count++;
  }

  EXPECT_EQ(count, 5);
}

TEST_F(ArchetypeChunkTest, FullChunkRejectsNewEntities) {
  for (uint64_t i = 1; i <= 64; ++i) {
    Entity e(i);
    auto result = m_chunk.add_entity(e);
    EXPECT_EQ(result.m_id, i);
  }

  EXPECT_EQ(m_chunk.size(), 64);

  Entity overflow(65);
  auto result = m_chunk.add_entity(overflow);
  EXPECT_EQ(result.m_id, 0);
  EXPECT_EQ(m_chunk.size(), 64);
}

TEST_F(ArchetypeChunkTest, IterateComponents) {
  for (uint64_t i = 1; i <= 5; ++i) {
    Entity e(i);
    m_chunk.add_entity(e);

    TestPositionComponent pos(i, static_cast<float>(i * 10), static_cast<float>(i * 20));
    m_chunk.set_component(e, pos);

    TestVelocityComponent vel(i, static_cast<float>(i), static_cast<float>(i * -1));
    m_chunk.set_component(e, vel);
  }

  float sum_x = 0;
  float sum_vx = 0;

  for (Entity e : m_chunk) {
    auto* pos = m_chunk.get_component<TestPositionComponent>(e);
    auto* vel = m_chunk.get_component<TestVelocityComponent>(e);

    if (pos && vel) {
      sum_x += pos->m_x;
      sum_vx += vel->m_vx;
    }
  }

  EXPECT_FLOAT_EQ(sum_x, 10.0f + 20.0f + 30.0f + 40.0f + 50.0f);
  EXPECT_FLOAT_EQ(sum_vx, 1.0f + 2.0f + 3.0f + 4.0f + 5.0f);
}

class SingleComponentChunkTest : public ::testing::Test {
protected:
  ArchetypeChunk<TestPositionComponent> m_chunk;
};

TEST_F(SingleComponentChunkTest, AddAndRetrieve) {
  Entity e(1);
  m_chunk.add_entity(e);

  TestPositionComponent pos(1, 100.0f, 200.0f);
  m_chunk.set_component(e, pos);

  auto* retrieved = m_chunk.get_component<TestPositionComponent>(e);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_FLOAT_EQ(retrieved->m_x, 100.0f);
  EXPECT_FLOAT_EQ(retrieved->m_y, 200.0f);
}