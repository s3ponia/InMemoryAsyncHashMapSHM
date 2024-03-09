#include "Server.hpp"

#include <cassert>
#include <iostream>

void Server::run() {
  const auto begin = std::chrono::steady_clock::now();
  while (true) {
    auto nextCommand = parseNextCommand();
    ++operation_counter_;
    switch (nextCommand.type) {
    case Command::INSERT:
      threadPool_.submitTask(
          [this, key = std::string{nextCommand.key},
           value = std::string{nextCommand.value},
           op_cnt = operation_counter_] { hashMap_.put(key, value, op_cnt); });
      break;
    case Command::DELETE:
      threadPool_.submitTask(
          [this, key = std::string{nextCommand.key},
           op_cnt = operation_counter_] { hashMap_.erase(key, op_cnt); });
      break;
    case Command::READ:
      threadPool_.submitTask([this, key = std::string{nextCommand.key}] {
        hashMap_.read(key);
        // std::cout << '"' << key << "\": \""
        //           << hashMap_.read(key).value_or("no value") << '"' << '\n';
      });
      break;
    case Command::STAT: {
      threadPool_.shutdown();
      const auto end = std::chrono::steady_clock::now();
      writeStat(begin, end);
      threadPool_.restore();
      break;
    }
    case Command::EXIT:
      threadPool_.shutdown();
      return;
    }
  }
}

void Server::writeStat(std::chrono::steady_clock::time_point begin,
                       std::chrono::steady_clock::time_point end) {
  const auto running_time =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin) -
      waiting_time_;

  std::cout << "Server handled " << operation_counter_ << " operations"
            << std::endl;

  std::cout << "Server running time without request waiting: "
            << running_time.count() << " ms" << std::endl;
}

void Server::incCursor(int incVal) {
  cursor_ = (cursor_ + incVal) % shared_memory_.size();
}

int Server::readInt() {
  std::string intStr{};
  while ('0' <= shared_memory_[cursor_] && shared_memory_[cursor_] <= '9') {
    intStr += shared_memory_[cursor_];
    incCursor();
  }

  if (intStr.empty()) {
    throw std::invalid_argument{"error in parsing int"};
  }

  return std::stoi(intStr);
}

std::string Server::readString(std::size_t sz) {
  if (cursor_ + sz < shared_memory_.size()) {
    const auto res = std::string{shared_memory_.substr(cursor_, sz)};
    cursor_ += sz;
    return res;
  } else {
    std::string res{};
    const auto svCursor = cursor_;
    res += shared_memory_.substr(cursor_);
    incCursor(sz);
    if (svCursor < cursor_) {
      throw std::out_of_range{"too large string"};
    }
    res += shared_memory_.substr(0, cursor_);
    return res;
  }
}

// format: I(keySize):(valueSize):(keyString)(ValueString)
auto Server::parseInsert() -> Command {
  assert(shared_memory_[cursor_] == 'I');
  incCursor();

  const auto keySize = readInt();
  assert(shared_memory_[cursor_] == ':');
  incCursor();

  const auto valueSize = readInt();
  assert(shared_memory_[cursor_] == ':');
  incCursor();

  auto keyString = readString(keySize);
  auto valueString = readString(valueSize);

  return Command{Command::INSERT, std::move(keyString), std::move(valueString)};
}

// format: R(keySize):(keyString)
auto Server::parseRead() -> Command {
  assert(shared_memory_[cursor_] == 'R');
  incCursor();

  const auto keySize = readInt();
  assert(shared_memory_[cursor_] == ':');
  incCursor();

  auto keyString = readString(keySize);

  return Command{Command::READ, std::move(keyString), {}};
}

// format: D(keySize):(keyString)
auto Server::parseDelete() -> Command {
  assert(shared_memory_[cursor_] == 'D');
  incCursor();

  const auto keySize = readInt();
  assert(shared_memory_[cursor_] == ':');
  incCursor();

  auto keyString = readString(keySize);

  return Command{Command::DELETE, std::move(keyString), {}};
}

// format: E
auto Server::parseExit() -> Command {
  assert(shared_memory_[cursor_] == 'E');
  incCursor();

  return Command{Command::EXIT, {}, {}};
}

// format: S
auto Server::parseStat() -> Command {
  assert(shared_memory_[cursor_] == 'S');
  incCursor();

  return Command{Command::STAT, {}, {}};
}

auto Server::parseNextCommand() -> Command {
  const auto begin = std::chrono::steady_clock::now();

  sem_wait(semaphore_); // wait client request

  const auto end = std::chrono::steady_clock::now();

  waiting_time_ +=
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

  switch (shared_memory_[cursor_]) {
  case 'I':
    return parseInsert();
  case 'D':
    return parseDelete();
  case 'R':
    return parseRead();
  case 'E':
    return parseExit();
  case 'S':
    return parseStat();
  }
  throw std::runtime_error{"Unhandled Command type"};
}