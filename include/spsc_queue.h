#ifndef SPSC_QUEUE_INCLUDE_SPSC_QUEUE_H_
#define SPSC_QUEUE_INCLUDE_SPSC_QUEUE_H_

#include <cstddef>
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
    if (push_count_ - pop_count_ == Capacity) return false;
    new (SlotAt(push_count_)) T(std::move(item));
    ++push_count_;
    return true;
  }

  bool TryPop(T& out) {
    if (push_count_ == pop_count_) return false;
    T* slot = SlotAt(pop_count_);
    out = std::move(*slot);
    slot->~T();
    ++pop_count_;
    return true;
  }

 private:
  T* SlotAt(std::size_t position) {
    std::size_t index = position & (Capacity - 1);
    return reinterpret_cast<T*>(storage_ + index * sizeof(T));
  }

  alignas(alignof(T)) std::byte storage_[Capacity * sizeof(T)];
  std::size_t push_count_{};
  std::size_t pop_count_{};
};

};  // namespace spsc_queue

#endif  // SPSC_QUEUE_INCLUDE_SPSC_QUEUE_H_