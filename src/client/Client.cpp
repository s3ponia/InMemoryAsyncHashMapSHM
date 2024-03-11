#include "Client.hpp"

Client::Client(char *shared_memory, std::size_t shared_memory_size)
    : shared_memory_(shared_memory),
      offset_in_shm_(
          (size_t *)(shared_memory + shared_memory_size - sizeof(std::size_t))),
      conn_semaphore_req_(connSemaphoreReq()),
      conn_semaphore_resp_(connSemaphoreResp()),
      conn_semaphore_rcv_(connSemaphoreRcv()) {}

Client::~Client() { running_ = false; }

std::optional<Connection> Client::connect() {
  sem_post(conn_semaphore_req_);
  sem_wait(conn_semaphore_resp_);
  const auto offset = *offset_in_shm_;
  sem_post(conn_semaphore_rcv_);

  if (offset == -1) {
    return std::nullopt;
  } else {
    auto res =
        Connection{offset, *(SharedMemoryBuff *)(shared_memory_ + offset)};

    std::clog << "Established connection with offset " << res.buffOffset()
              << std::endl;
    std::thread{[this, res]() mutable {
      while (running_) {
        res.waitPing();
      }
    }}.detach();

    return res;
  }
}
