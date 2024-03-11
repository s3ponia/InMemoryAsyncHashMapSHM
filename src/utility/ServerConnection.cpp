#include "ServerConnection.hpp"

ServerConnection::ServerConnection(std::size_t offset,
                                   SharedMemoryBuff &shm_buffer,
                                   HashMap &hashMap, ThreadPool &threadPool)
    : Connection(offset, shm_buffer), hashMap_(hashMap),
      threadPool_(threadPool) {}

bool ServerConnection::handleCommand() {
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
        responses_shm_.writeInsert(key, value);
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
        responses_shm_.writeErase(key);
      }
      sem_post(sem_resp_);
    });
    break;
  case Command::READ:
    threadPool_.submitTask([this, key = std::string{nextCommand->key}] {
      const auto value = hashMap_.read(key);

      std::cout << key << ": " << value.value_or("not found") << std::endl;

      {
        std::unique_lock lock{output_mutex_};

        if (value.has_value()) {
          responses_shm_.writeReadResponse(key, *value);
        } else {
          responses_shm_.writeReadEmptyResponse(key);
        }
      }

      sem_post(sem_resp_);
    });
    break;
  }
  return true;
}
