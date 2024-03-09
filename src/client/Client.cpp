#include "Client.hpp"

#include <string>

std::size_t Client::stringSize(std::size_t number) {
  return std::to_string(number).size();
}

// format: I(keySize):(valueSize):(key)(value)
void Client::insert(std::string_view key, std::string_view value) {
  const auto keySize = std::to_string(key.size());
  const auto valueSize = std::to_string(value.size());

  writeString("I");
  writeString(keySize);
  writeString(":");
  writeString(valueSize);
  writeString(":");
  writeString(key);
  writeString(value);

  sem_post(semaphore_);
}

void Client::writeString(std::string_view val) { buffer_.writeData(val); }

// format: D(keySize):(key)
void Client::erase(std::string_view key) {
  const auto keySize = std::to_string(key.size());

  writeString("D");
  writeString(keySize);
  writeString(":");
  writeString(key);

  sem_post(semaphore_);
}

// format: R(keySize):(key)
void Client::read(std::string_view key) {
  const auto keySize = std::to_string(key.size());

  writeString("R");
  writeString(keySize);
  writeString(":");
  writeString(key);

  sem_post(semaphore_);
}

// format: E
void Client::exit() {
  writeString("E");
  sem_post(semaphore_);
}

// format: S
void Client::stat() {
  writeString("S");
  sem_post(semaphore_);
}
