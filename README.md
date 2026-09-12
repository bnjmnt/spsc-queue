# spsc-queue

A lock-free, header-only single-producer single-consumer (SPSC) ring buffer for C++20.

## Design

- Fixed capacity, set at compile time via a template parameter (must be a power of two).
- Storage is a raw, aligned byte buffer. Items are constructed in place with placement `new` on push and explicitly destroyed on pop, so `T` does not need to be default-constructible.
- `push_count_` and `pop_count_` are monotonically increasing counters, each written by exactly one thread (producer and consumer respectively). Physical slot index is `count & (Capacity - 1)`.
- Each counter is padded to its own cache line (`alignas(std::hardware_destructive_interference_size)`) to avoid false sharing between the producer's and consumer's cores.
- Each side keeps a local, non-atomic cached copy of the other side's counter (`cached_pop_`, `cached_push_`), only falling back to a real atomic load when the cached value indicates full/empty. A stale cached value can only be conservative, never incorrect in the unsafe direction.
- Producer publishes with `memory_order_release` on `push_count_`; consumer synchronizes with `memory_order_acquire` on the same. Mirrored for `pop_count_`. Each side reads its own counter with `memory_order_relaxed`, since it is the only writer.

## Usage

```cpp
#include "spsc_queue.h"

spsc_queue::SpscQueue<int, 1024> q;

q.TryPush(42);

int out;
if (q.TryPop(out)) {
  // use out
}
```

`TryPush` and `TryPop` are non-blocking: they return `false` immediately if the queue is full or empty, rather than waiting.

Safe for exactly one producer thread and one consumer thread. Using more than one thread on either side is not supported and will corrupt state.

## Build

```
cmake -B build
cmake --build build
ctest --build-dir build --output-on-failure
```

## Status

Correctness (single-threaded ring logic, raw-buffer lifetime management, atomic memory ordering) is implemented and tested, including a two-thread stress test. Throughput benchmarking against a mutex-protected queue is not yet done.