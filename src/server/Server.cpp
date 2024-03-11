#include "Server.hpp"

#include <cstring>

#include "utility/SharedMemoryBuff.hpp"

void Server::runConnection(std::shared_ptr<Connection> conn) {
  conn->handleCommand();
  threadPool_.submitTask([conn = std::move(conn), this]() mutable {
    runConnection(std::move(conn));
  });
}

void Server::pingConnections() {
  for (auto it = connections_.begin(); it != connections_.end();) {
    if (!(*it)->ping()) {
      free_offsets_.push_back((*it)->buffOffset());
      it = connections_.erase(it);
    } else {
      ++it;
    }
  }
}

void Server::run() {
  while (true) {
    pingConnections();
    const auto conn = waitConnection();
    if (conn.has_value()) {
      connections_.push_back(*conn);
      threadPool_.submitTask(
          [conn = *conn, this]() mutable { runConnection(std::move(conn)); });
    }
  }
}

void Server::writeOffset(std::size_t off) {
  std::memcpy(shared_memory_ + shared_memory_size_ - sizeof(off), &off,
              sizeof(off));
}

void Server::fillOffsets() {
  for (std::size_t offset = 0;
       offset < shared_memory_size_ - sizeof(std::size_t);
       offset += sizeof(SharedMemoryBuff)) {
    free_offsets_.push_back(offset);
  }
}

std::shared_ptr<Connection> Server::initConnectionInShm(std::size_t off) {
  SharedMemoryBuff *buff = new (shared_memory_ + off) SharedMemoryBuff{};
  buff->offset = off;

  return std::make_shared<Connection>(*buff, hashMap_, threadPool_);
}

std::optional<std::shared_ptr<Connection>> Server::waitConnection() {
  sem_wait(conn_semaphore_req_);
  if (free_offsets_.empty()) {
    writeOffset(-1);
    sem_post(conn_semaphore_resp_);
    return std::nullopt;
  }
  const auto offset = free_offsets_.back();
  free_offsets_.pop_back();
  const auto connection = initConnectionInShm(offset);
  writeOffset(offset);
  sem_post(conn_semaphore_resp_);
  return connection;
}
