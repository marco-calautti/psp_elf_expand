#include <iostream>
#include <fstream>
#include <optional>
#include <elf_utils.h>

static std::optional<std::vector<uint8_t>> load_file(const char* filename){
    std::ifstream input(filename, std::ios::binary);

    if(!input){
        return std::nullopt;
    }

    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    std::vector<uint8_t> input_buffer(size);
    input.read((char*)input_buffer.data(),input_buffer.size());

    return std::make_optional(input_buffer);
}

int main(int argc, char* argv[]){
    if(argc != 3 && argc != 5){
        std::cout  << "Get virtual address of where a new segment will be added:\n"
                        << argv[0] << " "
                        << "vaddr <decrypted_eboot>\n"
                    << "Actually expand with given segment:\n"
                        << argv[0] << " "
                        << "expand <decrypted_eboot> <out_filename> <segment_binary_filename>\n";

        return 0;
    }
       
    std::string command(argv[1]);

    uint32_t new_segment_vaddr = 0;
    if(command == "vaddr" && argc == 3){
        auto elf_data = load_file(argv[2]);
        if(!elf_data.has_value()){
            std::cout << "Could not open file " << argv[1] << std::endl;
            return -1;
        }
        new_segment_vaddr = elf_new_segment_vaddr(elf_data.value());
    }else if(command == "expand" && argc == 5){
        auto elf_data = load_file(argv[2]);
        if(!elf_data.has_value()){
            std::cout << "Could not open file " << argv[1] << std::endl;
            return -1;
        }
        auto segment_data = load_file(argv[4]);
        if(!segment_data.has_value()){
            std::cout << "Could not open file " << argv[4] << std::endl;
            return -1;
        }
        new_segment_vaddr = elf_expand(elf_data.value(),segment_data.value());
        std::ofstream output(argv[3],std::ios::binary);
        if(!output){
            std::cout << "Could not open file " << argv[2] << std::endl;
            return -1;
        }

        output.write((char*)elf_data.value().data(),elf_data.value().size());
        std::cout << "New segment added!\n";
    }else{
        std::cout << "Unknown command " << command << " or wrong arguments." << std::endl;
        return -1;
    }
    
    std::cout << "Virtual ELF Address: 0x"
              << std::hex << new_segment_vaddr << std::endl
              << "Virtual Memory Address: 0x" <<
              std::hex << 0x8804000ULL + new_segment_vaddr << std::endl;
    return 0;
}
