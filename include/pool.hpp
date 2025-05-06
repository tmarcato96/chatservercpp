#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  explicit ThreadPool(size_t threadCount);
  ~ThreadPool();

  template <typename F> void enqueue(F &&f) {
    std::cout << "Task added\n";
    auto task = std::function<void()>(std::forward<F>(f));
    {
      std::lock_guard lock(queueMutex);
      tasks.emplace(std::move(task));
    }
    cv.notify_one();
  }

private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex queueMutex;
  std::condition_variable cv;
  std::atomic<bool> stop;
};