#pragma once

#include <fcntl.h>
#include <semaphore.h>

#include <thread>

#include "SharedMemoryBuff.hpp"

const auto SEM_NAME = "ShmHashMapSem";
const auto SEM_PERMS = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

const auto SHARED_MEMORY_OBJECT_NAME = "ShmHashMap";
const auto SHARED_MEMORY_OBJECT_SIZE =
    sizeof(std::size_t) +
    sizeof(SharedMemoryBuff) * std::thread::hardware_concurrency();

inline sem_t *connSemaphoreReq() {
  return sem_open("connSemaphoreReq", O_RDWR | O_CREAT, SEM_PERMS, 0);
}

inline sem_t *connSemaphoreResp() {
  return sem_open("connSemaphoreResp", O_RDWR | O_CREAT, SEM_PERMS, 0);
}

inline sem_t *connSemaphoreRcv() {
  return sem_open("connSemaphoreRcv", O_RDWR | O_CREAT, SEM_PERMS, 0);
}
