#pragma once
#include <windows.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

template<typename T, size_t N>
class LockFreeQueue {
    std::atomic<size_t> head{ 0 };
    std::atomic<size_t> tail{ 0 };
    T buffer[N];

public:
    bool push(const T& value) {
        size_t nextTail = (tail.load(std::memory_order_relaxed) + 1) % N;
        if (nextTail == head.load(std::memory_order_acquire)) return false; // full
        buffer[tail.load(std::memory_order_relaxed)] = value;
        tail.store(nextTail, std::memory_order_release);
        return true;
    }

    bool pop(T& value) {
        size_t currentHead = head.load(std::memory_order_relaxed);
        if (currentHead == tail.load(std::memory_order_acquire)) return false; // empty
        value = buffer[currentHead];
        head.store((currentHead + 1) % N, std::memory_order_release);
        return true;
    }

    void clear() {
        head.store(tail.load(std::memory_order_acquire), std::memory_order_release);
    }
};