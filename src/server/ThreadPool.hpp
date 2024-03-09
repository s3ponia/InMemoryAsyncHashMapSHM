#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  ThreadPool() {
    workers_.reserve(maxWorkers());
    for (std::size_t i = 0; i < maxWorkers(); ++i) {
      workers_.push_back(std::make_unique<Worker>(this));
    }
  }

  void submitTask(std::function<void()> func) {
    {
      std::lock_guard guard{mutex_};
      callbacks_.push(func);
    }

    if (callbacks_.size() >= maxWorkers())
      restore();
  }

  void restore() {
    for (auto &w : workers_) {
      w->restore();
    }
  }

  void shutdown() {
    for (auto &w : workers_) {
      w->shutdown();
    }
  }

  ~ThreadPool() { shutdown(); }

private:
  static std::size_t maxWorkers() {
    return std::thread::hardware_concurrency();
  }

  struct Worker {
    Worker(ThreadPool *pool) : pool_(pool), thread_() { restore(); }

    void restore() {
      bool prev{false};
      if (running_.compare_exchange_strong(prev, true)) {
        if (thread_.joinable())
          thread_.join();
        thread_ = std::thread{[this] { run(); }};
      }
    }

    void shutdown() {
      if (thread_.joinable())
        thread_.join();
    }

  private:
    void run() {
      while (runOnce()) {
      }
      running_ = false;
    }

    bool runOnce() {
      std::function<void()> funcToRun;
      {
        std::lock_guard guard{pool_->mutex_};
        if (!pool_->callbacks_.empty()) {
          funcToRun = pool_->callbacks_.front();
          pool_->callbacks_.pop();
        }
      }
      if (funcToRun) {
        funcToRun();
        return true;
      } else {
        return false;
      }
    }

    ThreadPool *pool_;
    std::thread thread_;
    std::atomic_bool running_{false};
  };

  std::queue<std::function<void()>> callbacks_;
  std::mutex mutex_;
  std::vector<std::unique_ptr<Worker>> workers_;
};
