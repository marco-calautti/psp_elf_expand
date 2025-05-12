#include <iostream>
#include <fstream>
#include <elf_utils.h>

int main(int argc, char* argv[]){
    if(argc != 4){
        std::cout << "Usage: <descrypted_eboot> <out_filename> <new_segment_size>\n";
        return 0;
    }
        
    std::ifstream input(argv[1], std::ios::binary);
    if(!input){
        std::cout << "Could not open file " << argv[1] << std::endl;
        return -1;
    }

    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    std::vector<uint8_t> input_buffer(size);
    input.read((char*)input_buffer.data(),input_buffer.size());

    uint32_t new_segment_size = std::atoi(argv[3]);
    uint32_t new_segment_vaddr = expand_elf(input_buffer, new_segment_size);

    std::ofstream output(argv[2],std::ios::binary);
    if(!output){
        std::cout << "Could not open file " << argv[2] << std::endl;
        return -1;
    }

    output.write((char*)input_buffer.data(),input_buffer.size());

    std::cout << "New segment added!\n"
              << "Virtual ELF Address: 0x"
              << std::hex << new_segment_vaddr << std::endl
              << "Virtual Memory Address: 0x" <<
              std::hex << 0x8804000ULL + new_segment_vaddr << std::endl;
    return 0;
}
