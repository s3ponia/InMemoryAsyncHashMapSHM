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
    enum { INSERT, READ, DELETE, EXIT } type;
    std::string key;
    std::string value;
  };

  void incCursor(int incVal = 1);

  int readInt();

  std::string readString(int sz);

  Command parseInsert();
  Command parseRead();
  Command parseDelete();
  Command parseExit();

  Command parseNextCommand();

  std::size_t operation_counter_{};

  int cursor_{};
  std::string_view shared_memory_;
  sem_t *semaphore_;

  std::chrono::milliseconds waiting_time_{};
  HashMap hashMap_;
  ThreadPool threadPool_;
};