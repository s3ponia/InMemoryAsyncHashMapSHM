#pragma once

#include <cstddef>
#include <string_view>

#include "utility/SharedMemoryBuff.hpp"

class CyclicBufferShm {
public:
  CyclicBufferShm(char *buffer, std::size_t buffer_size)
      : buffer_(buffer), buffer_size_(buffer_size) {}

  void writeReadEmptyResponse(std::string_view key);
  void writeReadResponse(std::string_view key, std::string_view value);
  void writeInsert(std::string_view key, std::string_view value);
  void writeErase(std::string_view key);
  void writeRead(std::string_view key);

  void writeData(std::string_view data);

  void eraseLastData(std::size_t sz) noexcept;

  std::size_t usedSpace() const noexcept;
  std::size_t freeSpace() const noexcept;

private:
  void incEnd(std::size_t val = 1);
  void decEnd(std::size_t val = 1);

  char *buffer_;
  std::size_t buffer_size_;

  std::size_t begin_{};
  std::size_t end_{};
};