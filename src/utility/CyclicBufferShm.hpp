#pragma once

#include <cstddef>
#include <optional>
#include <string_view>

#include "utility/SharedMemoryBuff.hpp"

struct Command {
  enum { INSERT, READ, DELETE } type;
  std::string key;
  std::string value;
};

class CyclicBufferShm {
public:
  CyclicBufferShm(char *buffer, std::size_t buffer_size)
      : buffer_(buffer), buffer_size_(buffer_size) {}

  std::optional<Command> parseNextCommandFromBegin();

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
  void incBegin(int incVal = 1);

  int readInt();

  std::string readString(std::size_t sz);

  Command parseInsert();
  Command parseRead();
  Command parseDelete();

  void incEnd(std::size_t val = 1);
  void decEnd(std::size_t val = 1);

  char *buffer_;
  std::size_t buffer_size_;

  std::size_t begin_{};
  std::size_t end_{};
};