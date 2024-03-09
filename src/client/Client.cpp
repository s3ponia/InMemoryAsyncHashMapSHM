#include "Client.hpp"

#include <string>

std::size_t Client::stringSize(std::size_t number) {
  return std::to_string(number).size();
}

// format: I(keySize):(valueSize):(key)(value)
void Client::insert(std::string_view key, std::string_view value) {
  bool commited = false;

  const auto keySize = std::to_string(key.size());
  const auto valueSize = std::to_string(value.size());

  auto guardType = WriteGuard{this, "I", commited};
  auto guardKeySize = WriteGuard{this, std::to_string(key.size()), commited};
  auto guardKeyDots = WriteGuard{this, ":", commited};
  auto guardValueSize =
      WriteGuard{this, std::to_string(value.size()), commited};
  auto guardValueDots = WriteGuard{this, ":", commited};
  auto guardKey = WriteGuard{this, key, commited};
  auto guardValue = WriteGuard{this, value, commited};

  guardType.write();
  guardKeySize.write();
  guardKeyDots.write();
  guardValueSize.write();
  guardValueDots.write();
  guardKey.write();
  guardValue.write();

  commited = true;
  sem_post(semaphore_);
}

void Client::writeString(std::string_view val) { buffer_.writeData(val); }

// format: D(keySize):(key)
void Client::erase(std::string_view key) {
  bool commited = false;

  const auto keySize = std::to_string(key.size());

  auto guardType = WriteGuard{this, "D", commited};
  auto guardKeySize = WriteGuard{this, std::to_string(key.size()), commited};
  auto guardKeyDots = WriteGuard{this, ":", commited};
  auto guardKey = WriteGuard{this, key, commited};

  guardType.write();
  guardKeySize.write();
  guardKeyDots.write();
  guardKey.write();

  commited = true;
  sem_post(semaphore_);
}

// format: R(keySize):(key)
void Client::read(std::string_view key) {
  bool commited = false;

  const auto keySize = std::to_string(key.size());

  auto guardType = WriteGuard{this, "R", commited};
  auto guardKeySize = WriteGuard{this, std::to_string(key.size()), commited};
  auto guardKeyDots = WriteGuard{this, ":", commited};
  auto guardKey = WriteGuard{this, key, commited};

  guardType.write();
  guardKeySize.write();
  guardKeyDots.write();
  guardKey.write();

  commited = true;
  sem_post(semaphore_);
}

// format: E
void Client::exit() {
  bool commited = false;
  auto guardType = WriteGuard{this, "E", commited};
  guardType.write();
  commited = true;
  sem_post(semaphore_);
}

// format: S
void Client::stat() {
  bool commited = false;
  auto guardType = WriteGuard{this, "S", commited};
  guardType.write();
  commited = true;
  sem_post(semaphore_);
}
