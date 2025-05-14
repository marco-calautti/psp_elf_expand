#pragma once

#include <elf_types.h>
#include <vector>
uint32_t elf_expand(std::vector<uint8_t>& elf, const std::vector<uint8_t>& new_segment);
uint32_t elf_new_segment_vaddr(std::vector<uint8_t>& elf);


