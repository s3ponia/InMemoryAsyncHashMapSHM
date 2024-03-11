#pragma once

#include <chrono>
#include <mutex>
#include <semaphore.h>
#include <stdexcept>
#include <string_view>

#include "HashMap.hpp"
#include "ThreadPool.hpp"
#include "utility/CyclicBufferShm.hpp"
#include "utility/SharedMemoryBuff.hpp"

class Connection {
public:
  Connection(SharedMemoryBuff &shm_buffer, HashMap &hashMap,
             ThreadPool &threadPool)
      : shm_buffer_(shm_buffer), hashMap_(hashMap), threadPool_(threadPool),
        input_(shm_buffer.requests_list.data(),
               shm_buffer.requests_list.size()),
        output_(shm_buffer.reponses_list.data(),
                shm_buffer.reponses_list.size()) {
    if ((sem_req_ = sem_open(semReqNameFromOffset(shm_buffer.offset).c_str(),
                             O_RDWR | O_CREAT, SEM_PERMS, 0)) == SEM_FAILED) {
      throw std::runtime_error{std::strerror(errno)};
    }

    if ((sem_resp_ = sem_open(semRespNameFromOffset(shm_buffer.offset).c_str(),
                              O_RDWR | O_CREAT, SEM_PERMS, 0)) == SEM_FAILED) {
      throw std::runtime_error{std::strerror(errno)};
    }

    if ((sem_ping_ = sem_open(semPingNameFromOffset(shm_buffer.offset).c_str(),
                              O_RDWR | O_CREAT, SEM_PERMS, 0)) == SEM_FAILED) {
      throw std::runtime_error{std::strerror(errno)};
    }
  }

  bool handleCommand();

  bool ping();

  std::size_t buffOffset();

private:
  std::optional<Command> parseNextCommand();

  sem_t *sem_req_;
  sem_t *sem_resp_;
  sem_t *sem_ping_;

  SharedMemoryBuff &shm_buffer_;
  std::mutex output_mutex_;

  HashMap &hashMap_;
  ThreadPool &threadPool_;

  CyclicBufferShm input_;
  CyclicBufferShm output_;
};