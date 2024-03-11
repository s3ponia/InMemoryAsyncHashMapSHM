#pragma once

#include <semaphore.h>

#include <string_view>

#include "utility/CyclicBufferShm.hpp"

class Client {
public:
  Client(CyclicBufferShm buffer, sem_t *semaphore)
      : buffer_(buffer), semaphore_(semaphore) {}

  void insert(std::string_view key, std::string_view value);
  void erase(std::string_view key);
  void read(std::string_view key);

private:
  std::size_t stringSize(std::size_t number);

  std::size_t dataSize() const;
  std::size_t freeSpace() const;

  CyclicBufferShm buffer_;
  sem_t *semaphore_;
};