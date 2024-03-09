#pragma once

#include <semaphore.h>

#include <string_view>

#include "CyclicBufferShm.hpp"

class Client {
public:
  Client(CyclicBufferShm buffer, sem_t *semaphore)
      : buffer_(buffer), semaphore_(semaphore) {}

  void insert(std::string_view key, std::string_view value);
  void erase(std::string_view key);
  void read(std::string_view key);
  void exit();
  void stat();

private:
  struct WriteGuard {
    WriteGuard(Client *client, std::string_view data, bool &completeFlag)
        : client_(client), data_(data), completeFlag_(completeFlag) {}

    void write() {
      client_->writeString(data_);
      data_written_ = true;
    }

    ~WriteGuard() {
      if (data_written_ && !completeFlag_)
        client_->buffer_.eraseLastData(data_.size());
    }

  private:
    Client *client_;
    std::string_view data_;
    bool data_written_{false};
    bool &completeFlag_;
  };

  void writeString(std::string_view val);

  std::size_t stringSize(std::size_t number);

  std::size_t dataSize() const;
  std::size_t freeSpace() const;

  CyclicBufferShm buffer_;
  sem_t *semaphore_;
};