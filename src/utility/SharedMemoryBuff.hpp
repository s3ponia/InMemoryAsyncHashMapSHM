#pragma once

#include <semaphore.h>

#include <array>
#include <string>

#include "utility/constants.hpp"

struct SharedMemoryBuff {
  std::size_t offset;
  std::array<char, SHARED_MEMORY_BUFF_SIZE> requests_list;
  std::array<char, SHARED_MEMORY_BUFF_SIZE> reponses_list;
};

inline std::string semReqNameFromOffset(std::size_t offset) {
  return std::string{"semConnReq"} + std::to_string(offset);
}

inline std::string semRespNameFromOffset(std::size_t offset) {
  return std::string{"semConnResp"} + std::to_string(offset);
}

inline std::string semPingNameFromOffset(std::size_t offset) {
  return std::string{"semPing"} + std::to_string(offset);
}
