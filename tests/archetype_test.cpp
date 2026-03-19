#include <gtest/gtest.h>

#include "archetype.hpp"

struct TestPos {
  uint64_t m_id;
  float m_x, m_y;

  TestPos() : m_id(0), m_x(0), m_y(0) {}
  TestPos(uint64_t id, float x, float y) : m_id(id), m_x(x), m_y(y) {}
  operator uint64_t() const { return m_id; }
};

struct TestVel {
  uint64_t m_id;
  float m_vx, m_vy;

  TestVel() : m_id(0), m_vx(0), m_vy(0) {}
  TestVel(uint64_t id, float vx, float vy) : m_id(id), m_vx(vx), m_vy(vy) {}
  operator uint64_t() const { return m_id; }
};

using TestArchetype = Archetype<TestPos, TestVel>;

class ArchetypeTest : public ::testing::Test {
protected:
  TestArchetype m_archetype;
};

TEST_F(ArchetypeTest, EmptyHasZeroEntities) {
  EXPECT_EQ(m_archetype.entity_count(), 0);
}

TEST_F(ArchetypeTest, CreateSingleEntity) {
  Entity e = m_archetype.create_entity();
  EXPECT_NE(e.m_id, 0);
  EXPECT_EQ(m_archetype.entity_count(), 1);
}

TEST_F(ArchetypeTest, CreateMultipleEntities) {
  for (int i = 0; i < 10; ++i) {
    Entity e = m_archetype.create_entity();
    EXPECT_NE(e.m_id, 0);
  }
  EXPECT_EQ(m_archetype.entity_count(), 10);
}

TEST_F(ArchetypeTest, CreateAndSetComponents) {
  Entity e = m_archetype.create_entity();

  TestPos pos(e.m_id, 10.0f, 20.0f);
  m_archetype.set_component(e, pos);

  TestVel vel(e.m_id, 5.0f, -3.0f);
  m_archetype.set_component(e, vel);

  auto* retrieved_pos = m_archetype.get_component<TestPos>(e);
  auto* retrieved_vel = m_archetype.get_component<TestVel>(e);

  ASSERT_NE(retrieved_pos, nullptr);
  ASSERT_NE(retrieved_vel, nullptr);

  EXPECT_FLOAT_EQ(retrieved_pos->m_x, 10.0f);
  EXPECT_FLOAT_EQ(retrieved_pos->m_y, 20.0f);
  EXPECT_FLOAT_EQ(retrieved_vel->m_vx, 5.0f);
  EXPECT_FLOAT_EQ(retrieved_vel->m_vy, -3.0f);
}

TEST_F(ArchetypeTest, DestroyEntity) {
  Entity e = m_archetype.create_entity();
  EXPECT_TRUE(m_archetype.contains(e));

  EXPECT_TRUE(m_archetype.destroy_entity(e));
  EXPECT_FALSE(m_archetype.contains(e));
  EXPECT_EQ(m_archetype.entity_count(), 0);
}

TEST_F(ArchetypeTest, DestroyNonExistentReturnsFalse) {
  Entity e(999);
  EXPECT_FALSE(m_archetype.destroy_entity(e));
}

TEST_F(ArchetypeTest, ForEachEntity) {
  for (int i = 0; i < 5; ++i) {
    m_archetype.create_entity();
  }

  int count = 0;
  m_archetype.for_each_entity([&count](Entity) { ++count; });

  EXPECT_EQ(count, 5);
}

TEST_F(ArchetypeTest, ForEachWithComponents) {
  for (int i = 0; i < 5; ++i) {
    Entity e = m_archetype.create_entity();

    TestPos pos(e.m_id, static_cast<float>(i * 10), static_cast<float>(i * 20));
    m_archetype.set_component(e, pos);

    TestVel vel(e.m_id, static_cast<float>(i), static_cast<float>(i * -1));
    m_archetype.set_component(e, vel);
  }

  float sum_x = 0;
  float sum_vx = 0;

  m_archetype.for_each([&sum_x, &sum_vx](Entity e, const TestPos& pos, const TestVel& vel) {
    sum_x += pos.m_x;
    sum_vx += vel.m_vx;
  });

  EXPECT_FLOAT_EQ(sum_x, 0 + 10 + 20 + 30 + 40);
  EXPECT_FLOAT_EQ(sum_vx, 0 + 1 + 2 + 3 + 4);
}

TEST_F(ArchetypeTest, CreateManyEntitiesSpansMultipleChunks) {
  size_t initial_chunks = m_archetype.chunk_count();

  for (int i = 0; i < 100; ++i) {
    m_archetype.create_entity();
  }

  EXPECT_GE(m_archetype.chunk_count(), initial_chunks);
  EXPECT_EQ(m_archetype.entity_count(), 100);
}

TEST_F(ArchetypeTest, MultipleEntitiesWithComponents) {
  for (int i = 0; i < 5; ++i) {
    Entity e = m_archetype.create_entity();

    TestPos pos(e.m_id, static_cast<float>(i * 10), static_cast<float>(i * 20));
    m_archetype.set_component(e, pos);

    TestVel vel(e.m_id, static_cast<float>(i), static_cast<float>(i * -1));
    m_archetype.set_component(e, vel);
  }

  float sum_x = 0;
  float sum_vx = 0;

  m_archetype.for_each_entity([this, &sum_x, &sum_vx](Entity e) {
    auto* pos = m_archetype.get_component<TestPos>(e);
    auto* vel = m_archetype.get_component<TestVel>(e);
    if (pos && vel) {
      sum_x += pos->m_x;
      sum_vx += vel->m_vx;
    }
  });

  EXPECT_FLOAT_EQ(sum_x, 0 + 10 + 20 + 30 + 40);
  EXPECT_FLOAT_EQ(sum_vx, 0 + 1 + 2 + 3 + 4);
}

class SingleComponentArchetypeTest : public ::testing::Test {
protected:
  Archetype<TestPos> m_archetype;
};

TEST_F(SingleComponentArchetypeTest, CreateAndRetrieve) {
  Entity e = m_archetype.create_entity();

  TestPos pos(e.m_id, 100.0f, 200.0f);
  m_archetype.set_component(e, pos);

  auto* retrieved = m_archetype.get_component<TestPos>(e);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_FLOAT_EQ(retrieved->m_x, 100.0f);
  EXPECT_FLOAT_EQ(retrieved->m_y, 200.0f);
}