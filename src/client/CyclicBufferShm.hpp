#pragma once

#include <cstddef>
#include <string_view>

class CyclicBufferShm {
public:
  CyclicBufferShm(char *buffer, std::size_t buffer_size)
      : buffer_(buffer), buffer_size_(buffer_size) {}

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