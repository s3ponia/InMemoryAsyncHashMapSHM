#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string_view>

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

    while (true) {
      try {
        const std::string command =
            readStr("Enter operation: read|insert|delete|exit: ");

        if (command == "read") {
          const auto key = readStr("Enter key: ");
          client.read(key);
        } else if (command == "insert") {
          const auto key = readStr("Enter key: ");
          const auto val = readStr("Enter value: ");
          client.insert(key, val);
        } else if (command == "delete") {
          const auto key = readStr("Enter key: ");
          client.erase(key);
        } else if (command == "exit") {
          client.exit();
          return 0;
        } else {
          std::cerr << "invalid operation: " << command << std::endl;
        }
      } catch (const std::exception &e) {
        std::cerr << "error in operation: " << e.what() << std::endl;
      }
    }
  } else {
    exit(EXIT_FAILURE);
  }
}