#include "CyclicBufferShm.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>

std::size_t CyclicBufferShm::usedSpace() const noexcept {
  if (begin_ < end_) {
    return end_ - begin_;
  } else {
    return (buffer_size_ - begin_) + end_;
  }
}

void CyclicBufferShm::writeReadEmptyResponse(std::string_view key) {
  const auto keySize = std::to_string(key.size());

  writeData("E");
  writeData(keySize);
  writeData(":");
  writeData(key);
}

void CyclicBufferShm::incBegin(int incVal) {
  begin_ = (begin_ + incVal) % buffer_size_;
}

int CyclicBufferShm::readInt() {
  std::string intStr{};
  while ('0' <= buffer_[begin_] && buffer_[begin_] <= '9') {
    intStr += buffer_[begin_];
    incBegin();
  }

  if (intStr.empty()) {
    throw std::invalid_argument{"error in parsing int"};
  }

  return std::stoi(intStr);
}

std::string CyclicBufferShm::readString(std::size_t sz) {
  if (begin_ + sz < buffer_size_) {
    const auto res = std::string{buffer_ + begin_, sz};
    begin_ += sz;
    return res;
  } else {
    std::string res{};
    const auto svCursor = begin_;
    res += std::string{buffer_ + begin_, sz};
    incBegin(sz);
    if (svCursor < begin_) {
      throw std::out_of_range{"too large string"};
    }
    res += std::string{buffer_, begin_};
    return res;
  }
}

// format: I(keySize):(valueSize):(keyString)(ValueString)
auto CyclicBufferShm::parseInsert() -> Command {
  assert(buffer_[begin_] == 'I');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  const auto valueSize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);
  auto valueString = readString(valueSize);

  return Command{Command::INSERT, std::move(keyString), std::move(valueString)};
}

// format: R(keySize):(keyString)
auto CyclicBufferShm::parseRead() -> Command {
  assert(buffer_[begin_] == 'R');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);

  return Command{Command::READ, std::move(keyString), {}};
}

// format: D(keySize):(keyString)
auto CyclicBufferShm::parseDelete() -> Command {
  assert(buffer_[begin_] == 'D');
  incBegin();

  const auto keySize = readInt();
  assert(buffer_[begin_] == ':');
  incBegin();

  auto keyString = readString(keySize);

  return Command{Command::DELETE, std::move(keyString), {}};
}

auto CyclicBufferShm::parseNextCommandFromBegin() -> std::optional<Command> {
  switch (buffer_[begin_]) {
  case 'I':
    return parseInsert();
  case 'D':
    return parseDelete();
  case 'R':
    return parseRead();
  }
  throw std::runtime_error{std::string{"Unhandled Command type: "} +
                           buffer_[begin_]};
}

void CyclicBufferShm::writeReadResponse(std::string_view key,
                                        std::string_view value) {
  const auto keySize = std::to_string(key.size());
  const auto valueSize = std::to_string(value.size());

  writeData("R");
  writeData(keySize);
  writeData(":");
  writeData(valueSize);
  writeData(":");
  writeData(key);
  writeData(value);
}

void CyclicBufferShm::writeInsert(std::string_view key,
                                  std::string_view value) {
  const auto keySize = std::to_string(key.size());
  const auto valueSize = std::to_string(value.size());

  writeData("I");
  writeData(keySize);
  writeData(":");
  writeData(valueSize);
  writeData(":");
  writeData(key);
  writeData(value);
}

void CyclicBufferShm::writeErase(std::string_view key) {
  const auto keySize = std::to_string(key.size());

  writeData("D");
  writeData(keySize);
  writeData(":");
  writeData(key);
}

void CyclicBufferShm::writeRead(std::string_view key) {
  const auto keySize = std::to_string(key.size());

  writeData("R");
  writeData(keySize);
  writeData(":");
  writeData(key);
}

std::size_t CyclicBufferShm::freeSpace() const noexcept {
  return buffer_size_ - usedSpace() - 1;
}

void CyclicBufferShm::incEnd(std::size_t val) {
  end_ = (end_ + val) % buffer_size_;
}

void CyclicBufferShm::decEnd(std::size_t val) {
  if (end_ > val) {
    end_ -= val;
  } else {
    val -= end_;
    end_ = buffer_size_ - val;
  }
}

void CyclicBufferShm::eraseLastData(std::size_t sz) noexcept {
  assert(sz < usedSpace());
  decEnd(sz);
}

void CyclicBufferShm::writeData(std::string_view data) {
  if (freeSpace() < data.size()) {
    throw std::out_of_range{"out of space in shared memory"};
  }

  if (end_ + data.size() <= buffer_size_) {
    std::memcpy(buffer_ + end_, data.data(), data.size());
    incEnd(data.size());
  } else {
    const auto sz = data.size();
    std::memcpy(buffer_ + end_, data.data(), buffer_size_ - end_);
    data.remove_prefix(buffer_size_ - end_);
    std::memcpy(buffer_, data.data(), data.size());
    incEnd(sz);
  }
}
