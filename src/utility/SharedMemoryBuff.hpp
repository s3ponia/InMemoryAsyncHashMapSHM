#pragma once

#include <semaphore.h>

#include <array>
#include <string>

const auto SHARED_MEMORY_BUFF_SIZE = (10 << 20);

struct SharedMemoryBuff {
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

inline std::string semPingRespNameFromOffset(std::size_t offset) {
  return std::string{"semPingResp"} + std::to_string(offset);
}