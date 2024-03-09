#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <string_view>
#include <thread>

#include "utility/constants.hpp"

#include "Client.hpp"
#include "CyclicBufferShm.hpp"

int initShm() {
  if (int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_RDWR, 0777); shm != -1) {
    return shm;
  } else {
    perror("shm_open");
    return -1;
  }
}

std::string readStr(std::string line) {
  std::cout << line;
  std::string data;
  std::getline(std::cin, data);
  return data;
}

std::string random_string(size_t length) {
  auto randchar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

void testInsertPerformance(Client &client, int insertCounts) {
  for (int i = 0; i < insertCounts; ++i) {
    client.insert(random_string(5), random_string(5));
  }

  std::cout << insertCounts << " inserts" << std::endl;
}

void testReadPerformance(sem_t *semaphore, Client &client, int insertCounts) {
  for (int i = 0; i < insertCounts; ++i) {
    client.read(random_string(5));
  }
  std::cout << insertCounts << " reads" << std::endl;
}

void testDeletePerformance(sem_t *semaphore, Client &client, int insertCounts) {
  for (int i = 0; i < insertCounts; ++i) {
    client.erase(random_string(5));
  }
  std::cout << insertCounts << " deletes" << std::endl;
}

int main() {
  sem_t *semaphore = sem_open(SEM_NAME, O_RDWR, SEM_PERMS, 0);
  if (semaphore == SEM_FAILED) {
    perror("sem_open(3) failed");
    exit(EXIT_FAILURE);
  }

  if (auto shm = initShm(); shm != -1) {
    char *addr = (char *)mmap(0, SHARED_MEMORY_OBJECT_SIZE + 1,
                              PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);

    CyclicBufferShm shared_memory{addr, SHARED_MEMORY_OBJECT_SIZE};
    Client client{shared_memory, semaphore};

    const auto operationsNumber = 1'000'000;

    testInsertPerformance(client, operationsNumber);
    testReadPerformance(semaphore, client, operationsNumber);
    testDeletePerformance(semaphore, client, operationsNumber);
    client.exit();
  } else {
    exit(EXIT_FAILURE);
  }
}