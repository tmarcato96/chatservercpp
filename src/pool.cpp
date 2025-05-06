#include <pool.hpp>

ThreadPool::ThreadPool(size_t threadCount) : stop(false) {
  for (size_t i = 0; i < threadCount; ++i) {

    workers.emplace_back([this]() {
      while (true) {

        std::function<void()> task;
        {
          std::unique_lock lock(queueMutex);
          cv.wait(lock, [this]() { return stop || !tasks.empty(); });

          if (stop && tasks.empty())
            return;

          task = std::move(tasks.front());
          tasks.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  stop = true;
  cv.notify_all();

  for (auto &worker : workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}
