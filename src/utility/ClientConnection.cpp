#include "ClientConnection.hpp"

#include <cassert>

std::future<void> ClientConnection::insert(std::string_view key,
                                           std::string_view value) {
  const auto id = reqId_++;
  std::promise<void> promise;
  std::future<void> res = promise.get_future();

  {
    std::lock_guard lock{mutex_};
    complete_promises_func_[id] =
        [p = std::make_shared<std::promise<void>>(std::move(promise))](
            auto &&) { p->set_value(); };
  }

  Connection::insert(id, key, value);
  return res;
}

std::future<void> ClientConnection::erase(std::string_view key) {
  const auto id = reqId_++;
  std::promise<void> promise;
  std::future<void> res = promise.get_future();

  {
    std::lock_guard lock{mutex_};
    complete_promises_func_[id] =
        [p = std::make_shared<std::promise<void>>(std::move(promise))](
            auto &&) { p->set_value(); };
  }

  Connection::erase(id, key);
  return res;
}

std::future<std::optional<std::string>>
ClientConnection::read(std::string_view key) {
  const auto id = reqId_++;
  std::promise<std::optional<std::string>> promise;
  std::future<std::optional<std::string>> res = promise.get_future();

  {
    std::lock_guard lock{mutex_};
    complete_promises_func_[id] =
        [p = std::make_shared<std::promise<std::optional<std::string>>>(
             std::move(promise))](const Response &resp) {
          if (resp.type == Response::READ_EMPTY)
            p->set_value(std::nullopt);
          else
            p->set_value(resp.value);
        };
  }

  Connection::read(id, key);
  return res;
}

void ClientConnection::readResponses() {
  while (running_) {
    sem_wait(sem_resp_);
    const auto resp = responses_shm_.parseResponseFromBegin();
    assert(resp.has_value());
    {
      std::lock_guard lock{mutex_};
      complete_promises_func_.at(resp->id)(*resp);
    }
  }
}
