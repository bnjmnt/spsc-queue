#include "spsc_queue.h"

#include <gtest/gtest.h>

#include <thread>

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

TEST(SpscQueueTest, ConcurrentPushPop) {
  constexpr int kItems = 1'000'000;
  SpscQueue<int, 1024> q;
  std::vector<int> received;
  received.reserve(kItems);

  std::thread producer([&] {
    for (int i = 0; i < kItems; ++i) {
      while (!q.TryPush(i)) {
      }
    }
  });

  std::thread consumer([&] {
    int out = 0;
    while (static_cast<int>(received.size()) < kItems) {
      if (q.TryPop(out)) received.push_back(out);
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(received.size(), static_cast<size_t>(kItems));
  for (int i = 0; i < kItems; ++i) {
    EXPECT_EQ(received[i], i);
  }
}

}  // namespace
}  // namespace spsc_queue