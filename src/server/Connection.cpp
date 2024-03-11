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

auto Connection::parseNextCommand() -> std::optional<Command> {
  if (sem_trywait(sem_req_) != 0)
    return std::nullopt;

  return input_.parseNextCommandFromBegin();
}
