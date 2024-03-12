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
                            reqId = nextCommand->id,
                            op_cnt = static_cast<std::size_t>(time(NULL))] {
      hashMap_.put(key, value, op_cnt);
      {
        std::unique_lock lock{output_mutex_};
        responses_shm_.writeInsert(reqId, key, value);
      }
      sem_post(sem_resp_);
    });
    break;
  case Command::DELETE:
    threadPool_.submitTask([this, key = std::string{nextCommand->key},
                            reqId = nextCommand->id,
                            op_cnt = static_cast<std::size_t>(time(NULL))] {
      hashMap_.erase(key, op_cnt);

      {
        std::unique_lock lock{output_mutex_};
        responses_shm_.writeErase(reqId, key);
      }
      sem_post(sem_resp_);
    });
    break;
  case Command::READ:
    threadPool_.submitTask(
        [this, key = std::string{nextCommand->key}, reqId = nextCommand->id] {
          const auto value = hashMap_.read(key);

          {
            std::unique_lock lock{output_mutex_};

            if (value.has_value()) {
              responses_shm_.writeReadResponse(reqId, key, *value);
            } else {
              responses_shm_.writeReadEmptyResponse(reqId, key);
            }
          }

          sem_post(sem_resp_);
        });
    break;
  default:
    break;
  }
  return true;
}
