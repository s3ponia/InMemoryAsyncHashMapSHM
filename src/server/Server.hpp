#pragma once

#include <chrono>
#include <semaphore.h>
#include <string_view>

#include "HashMap.hpp"
#include "ThreadPool.hpp"

class Server {
public:
  Server(std::string_view shared_memory, sem_t *semaphore,
         std::size_t hashMapSize)
      : shared_memory_(shared_memory), semaphore_(semaphore),
        hashMap_(hashMapSize) {}

  void run();

private:
  struct Command {
    enum { INSERT, READ, DELETE, STAT, EXIT } type;
    std::string key;
    std::string value;
  };

  void incCursor(int incVal = 1);

  int readInt();

  std::string readString(std::size_t sz);

  Command parseInsert();
  Command parseRead();
  Command parseDelete();
  Command parseExit();
  Command parseStat();

  void writeStat(std::chrono::steady_clock::time_point begin,
                 std::chrono::steady_clock::time_point end);

  Command parseNextCommand();

  std::size_t operation_counter_{};

  std::size_t cursor_{};
  std::string_view shared_memory_;
  sem_t *semaphore_;

  std::chrono::milliseconds waiting_time_{};
  HashMap hashMap_;
  ThreadPool threadPool_;
};
