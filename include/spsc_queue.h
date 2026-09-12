#ifndef SPSC_QUEUE_INCLUDE_SPSC_QUEUE_H_
#define SPSC_QUEUE_INCLUDE_SPSC_QUEUE_H_

#include <atomic>
#include <cstddef>
#include <new>
#include <utility>

namespace spsc_queue {

template <typename T, std::size_t Capacity>
class SpscQueue {
  static_assert(Capacity > 0 && (Capacity & (Capacity - 1)) == 0,
                "Capacity must be a power of two.");

 public:
  SpscQueue() = default;

  ~SpscQueue() {
    while (pop_count_ != push_count_) {
      SlotAt(pop_count_)->~T();
      ++pop_count_;
    }
  }

  SpscQueue(const SpscQueue&) = delete;
  SpscQueue& operator=(const SpscQueue&) = delete;

  bool TryPush(T item) {
    std::size_t push = push_count_.load(std::memory_order_relaxed);
    std::size_t pop = pop_count_.load(std::memory_order_acquire);
    if (push - pop == Capacity) {
      return false;
    }

    new (SlotAt(push)) T(std::move(item));
    push_count_.store(push + 1, std::memory_order_release);
    return true;
  }

  bool TryPop(T& out) {
    std::size_t pop = pop_count_.load(std::memory_order_relaxed);
    std::size_t push = push_count_.load(std::memory_order_acquire);
    if (push == pop) {
      return false;
    }

    T* slot = SlotAt(pop);
    out = std::move(*slot);
    slot->~T();
    pop_count_.store(pop + 1, std::memory_order_release);
    return true;
  }

 private:
  T* SlotAt(std::size_t position) {
    std::size_t index = position & (Capacity - 1);
    return reinterpret_cast<T*>(storage_ + index * sizeof(T));
  }

  alignas(alignof(T)) std::byte storage_[Capacity * sizeof(T)];
  alignas(std::hardware_destructive_interference_size)
      std::atomic<size_t> push_count_{0};
  alignas(std::hardware_destructive_interference_size)
      std::atomic<std::size_t> pop_count_{0};
};

};  // namespace spsc_queue

#endif  // SPSC_QUEUE_INCLUDE_SPSC_QUEUE_H_