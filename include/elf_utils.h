#pragma once

#include <elf_types.h>
#include <vector>
uint32_t expand_elf(std::vector<uint8_t>& elf, uint32_t new_segment_size);

