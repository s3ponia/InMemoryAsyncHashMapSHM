#pragma once

#include <fcntl.h>

const auto SEM_NAME = "ShmHashMapSem";
const auto SEM_PERMS = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

const auto SHARED_MEMORY_OBJECT_NAME = "ShmHashMap";
const auto SHARED_MEMORY_OBJECT_SIZE = 5ull * 1ull << 30ull;

const auto SHARED_MEMORY_BUFF_SIZE = (1 << 30); // 1GiB
