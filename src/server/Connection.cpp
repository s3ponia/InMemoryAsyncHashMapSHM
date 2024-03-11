#include "Connection.hpp"

#include <cassert>
#include <iostream>
#include <thread>

std::size_t Connection::buffOffset() { return shm_buffer_.offset; }

bool Connection::ping() {
  sem_post(sem_ping_);
  errno = 0;
  for (int i = 0; i < 5; ++i) {
    sem_trywait(sem_ping_);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  return errno == EAGAIN;
}

bool Connection::handleCommand() {
  auto nextCommand = parseNextCommand();
  if (!nextCommand.has_value()) {
    return false;
  }
  switch (nextCommand->type) {
  case Command::INSERT:
    threadPool_.submitTask([this, key = std::string{nextCommand->key},
                            value = std::string{nextCommand->value},
                            op_cnt = static_cast<std::size_t>(time(NULL))] {
      hashMap_.put(key, value, op_cnt);
      {
        std::unique_lock lock{output_mutex_};
        output_.writeInsert(key, value);
      }
      sem_post(sem_resp_);
    });
    break;
  case Command::DELETE:
    threadPool_.submitTask([this, key = std::string{nextCommand->key},
                            op_cnt = static_cast<std::size_t>(time(NULL))] {
      hashMap_.erase(key, op_cnt);

      {
        std::unique_lock lock{output_mutex_};
        output_.writeErase(key);
      }
      sem_post(sem_resp_);
    });
    break;
  case Command::READ:
    threadPool_.submitTask([this, key = std::string{nextCommand->key}] {
      const auto value = hashMap_.read(key);

      {
        std::unique_lock lock{output_mutex_};

        if (value.has_value()) {
          output_.writeReadResponse(key, *value);
        } else {
          output_.writeReadEmptyResponse(key);
        }
      }

      sem_post(sem_resp_);
    });
    break;
  }
  return true;
}

void Connection::incCursor(int incVal) {
  cursor_ = (cursor_ + incVal) % shm_buffer_.requests_list.size();
}

int Connection::readInt() {
  std::string intStr{};
  while ('0' <= shm_buffer_.requests_list[cursor_] &&
         shm_buffer_.requests_list[cursor_] <= '9') {
    intStr += shm_buffer_.requests_list[cursor_];
    incCursor();
  }

  if (intStr.empty()) {
    throw std::invalid_argument{"error in parsing int"};
  }

  return std::stoi(intStr);
}

std::string Connection::readString(std::size_t sz) {
  if (cursor_ + sz < shm_buffer_.requests_list.size()) {
    const auto res =
        std::string{shm_buffer_.requests_list.data() + cursor_, sz};
    cursor_ += sz;
    return res;
  } else {
    std::string res{};
    const auto svCursor = cursor_;
    res += std::string{shm_buffer_.requests_list.data() + cursor_, sz};
    incCursor(sz);
    if (svCursor < cursor_) {
      throw std::out_of_range{"too large string"};
    }
    res += std::string{shm_buffer_.requests_list.data(), cursor_};
    return res;
  }
}

// format: I(keySize):(valueSize):(keyString)(ValueString)
auto Connection::parseInsert() -> Command {
  assert(shm_buffer_.requests_list[cursor_] == 'I');
  incCursor();

  const auto keySize = readInt();
  assert(shm_buffer_.requests_list[cursor_] == ':');
  incCursor();

  const auto valueSize = readInt();
  assert(shm_buffer_.requests_list[cursor_] == ':');
  incCursor();

  auto keyString = readString(keySize);
  auto valueString = readString(valueSize);

  return Command{Command::INSERT, std::move(keyString), std::move(valueString)};
}

// format: R(keySize):(keyString)
auto Connection::parseRead() -> Command {
  assert(shm_buffer_.requests_list[cursor_] == 'R');
  incCursor();

  const auto keySize = readInt();
  assert(shm_buffer_.requests_list[cursor_] == ':');
  incCursor();

  auto keyString = readString(keySize);

  return Command{Command::READ, std::move(keyString), {}};
}

// format: D(keySize):(keyString)
auto Connection::parseDelete() -> Command {
  assert(shm_buffer_.requests_list[cursor_] == 'D');
  incCursor();

  const auto keySize = readInt();
  assert(shm_buffer_.requests_list[cursor_] == ':');
  incCursor();

  auto keyString = readString(keySize);

  return Command{Command::DELETE, std::move(keyString), {}};
}

auto Connection::parseNextCommand() -> std::optional<Command> {
  if (sem_trywait(sem_req_) != 0)
    return std::nullopt;

  switch (shm_buffer_.requests_list[cursor_]) {
  case 'I':
    return parseInsert();
  case 'D':
    return parseDelete();
  case 'R':
    return parseRead();
  }
  throw std::runtime_error{std::string{"Unhandled Command type: "} +
                           shm_buffer_.requests_list[cursor_]};
}
