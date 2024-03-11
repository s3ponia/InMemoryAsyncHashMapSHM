#pragma once

#include <chrono>
#include <memory>
#include <semaphore.h>
#include <string_view>

#include "Connection.hpp"
#include "HashMap.hpp"
#include "ThreadPool.hpp"

class Server {
public:
  Server(char *shared_memory, sem_t *conn_semaphore_req,
         sem_t *conn_semaphore_resp, std::size_t hashMapSize)
      : shared_memory_(shared_memory), conn_semaphore_req_(conn_semaphore_req),
        conn_semaphore_resp_(conn_semaphore_resp), hashMap_(hashMapSize) {
    fillOffsets();
  }

  void run();

private:
  void runConnection(std::shared_ptr<Connection> conn);
  void pingConnections();

  void fillOffsets();

  std::optional<std::shared_ptr<Connection>> waitConnection();
  void writeOffset(std::size_t off);

  std::shared_ptr<Connection> initConnectionInShm(std::size_t off);

  char *shared_memory_;
  std::size_t shared_memory_size_;

  sem_t *conn_semaphore_req_;
  sem_t *conn_semaphore_resp_;

  std::list<std::shared_ptr<Connection>> connections_;
  std::vector<std::size_t> free_offsets_;

  HashMap hashMap_;
  ThreadPool threadPool_;
};
