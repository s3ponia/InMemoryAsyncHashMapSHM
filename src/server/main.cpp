#include <iostream>

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "Server.hpp"
#include "utility/constants.hpp"

int initShm() {
  if (int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_CREAT | O_RDONLY, 0777);
      shm != -1) {
    if (ftruncate(shm, SHARED_MEMORY_OBJECT_SIZE + 1) == -1) {
      perror("ftruncate");
      shm_unlink(SHARED_MEMORY_OBJECT_NAME);
      return -1;
    }
    return shm;
  } else {
    perror("shm_open");
    return -1;
  }
}

void unlinkShm() { shm_unlink(SHARED_MEMORY_OBJECT_NAME); }
void unlinkSem() { sem_unlink(SEM_NAME); }
void atExit() {
  unlinkSem();
  unlinkShm();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " (hashmap size)" << std::endl;
    exit(EXIT_FAILURE);
  }

  unsigned long hashMapSize{};
  try {
    hashMapSize = std::stoul(argv[1]);
  } catch (const std::exception &ex) {
    std::cerr << "first argument: " << ex.what() << std::endl;
    std::cout << "usage: " << argv[0] << " (hashmap size)" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::atexit(atExit);

  sem_t *semaphore =
      sem_open(SEM_NAME, O_CREAT | O_EXCL | O_RDWR, SEM_PERMS, 0);

  if (semaphore == SEM_FAILED) {
    perror("sem_open(3) error");
    exit(EXIT_FAILURE);
  }

  if (auto shm = initShm(); shm != -1) {
    char *addr = (char *)mmap(0, SHARED_MEMORY_OBJECT_SIZE + 1, PROT_READ,
                              MAP_SHARED, shm, 0);
    std::string_view shared_memory(addr, SHARED_MEMORY_OBJECT_SIZE);
    Server server{shared_memory, semaphore, hashMapSize};
    server.run();
  } else {
    exit(EXIT_FAILURE);
  }
}