#include "CyclicBufferShm.hpp"

#include <cassert>

std::size_t CyclicBufferShm::usedSpace() const noexcept {
  if (begin_ < end_) {
    return end_ - begin_;
  } else {
    return (buffer_size_ - begin_) + end_;
  }
}

std::size_t CyclicBufferShm::freeSpace() const noexcept {
  return buffer_size_ - usedSpace() - 1;
}

void CyclicBufferShm::incEnd(std::size_t val) {
  end_ = (end_ + val) % buffer_size_;
}

void CyclicBufferShm::decEnd(std::size_t val) {
  if (end_ > val) {
    end_ -= val;
  } else {
    val -= end_;
    end_ = buffer_size_ - val;
  }
}

void CyclicBufferShm::eraseLastData(std::size_t sz) noexcept {
  assert(sz < usedSpace());
  decEnd(sz);
}

void CyclicBufferShm::writeData(std::string_view data) {
  if (freeSpace() < data.size()) {
    throw std::out_of_range{"out of space in shared memory"};
  }

  if (end_ + data.size() <= buffer_size_) {
    std::memcpy(buffer_ + end_, data.data(), data.size());
    incEnd(data.size());
  } else {
    const auto sz = data.size();
    std::memcpy(buffer_ + end_, data.data(), buffer_size_ - end_);
    data.remove_prefix(buffer_size_ - end_);
    std::memcpy(buffer_, data.data(), data.size());
    incEnd(sz);
  }
}
