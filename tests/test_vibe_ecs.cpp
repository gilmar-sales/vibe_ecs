#include <gtest/gtest.h>

#include "vibe-ecs.h"

TEST(SaudaoTest, RetornaTextoCorreto) {
    EXPECT_STREQ(vibe_version().c_str(), "0.0.1");
}
