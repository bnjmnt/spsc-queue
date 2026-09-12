#include <gtest/gtest.h>

#include "spsc_queue.h"

namespace spsc_queue {
namespace {

TEST(SpscQueueTest, PushPopInOrder) {
  SpscQueue<int, 4> q;
  EXPECT_TRUE(q.TryPush(1));
  EXPECT_TRUE(q.TryPush(2));

  int out = 0;
  EXPECT_TRUE(q.TryPop(out));
  EXPECT_EQ(out, 1);
  EXPECT_TRUE(q.TryPop(out));
  EXPECT_EQ(out, 2);
}

TEST(SpscQueueTest, PopEmptyFails) {
  SpscQueue<int, 4> q;
  int out = 0;
  EXPECT_FALSE(q.TryPop(out));
}

TEST(SpscQueueTest, PushPastCapacityFails) {
  SpscQueue<int, 4> q;
  for (int i = 0; i < 4; ++i) EXPECT_TRUE(q.TryPush(i));
  EXPECT_FALSE(q.TryPush(99));
}

TEST(SpscQueueTest, WrapsAroundTwice) {
  SpscQueue<int, 4> q;
  int out = 0;
  for (int cycle = 0; cycle < 3; ++cycle) {
    for (int i = 0; i < 4; ++i) EXPECT_TRUE(q.TryPush(cycle * 4 + i));
    for (int i = 0; i < 4; ++i) {
      EXPECT_TRUE(q.TryPop(out));
      EXPECT_EQ(out, cycle * 4 + i);
    }
  }
}

}  // namespace
}  // namespace spsc_queue