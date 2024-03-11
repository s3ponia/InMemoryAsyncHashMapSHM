#include "Client.hpp"

#include <string>

std::size_t Client::stringSize(std::size_t number) {
  return std::to_string(number).size();
}

// format: I(keySize):(valueSize):(key)(value)
void Client::insert(std::string_view key, std::string_view value) {
  buffer_.writeInsert(key, value);

  sem_post(semaphore_);
}

// format: D(keySize):(key)
void Client::erase(std::string_view key) {
  buffer_.writeErase(key);

  sem_post(semaphore_);
}

// format: R(keySize):(key)
void Client::read(std::string_view key) {
  buffer_.writeRead(key);

  sem_post(semaphore_);
}
