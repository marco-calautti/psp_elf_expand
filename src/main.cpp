#include <elfio/elfio.hpp>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <optional>

#define ALIGN 0x40
#define SEGMENT_SIZE 0x10

static ELFIO::Elf32_Addr align(ELFIO::Elf32_Addr value, size_t align){
    return (value + (align-1))/align*align;
}

static void print_vaddr(ELFIO::Elf32_Off vaddr){
     std::cout << "Virtual ELF Address: 0x"
              << std::hex << vaddr << std::endl
              << "Virtual Memory Address: 0x" <<
              std::hex << 0x8804000ULL + vaddr << std::endl;
}

static std::optional<std::vector<char>> load_file(const char* filename){
    std::ifstream input(filename, std::ios::binary);

    if(!input){
        return std::nullopt;
    }

    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    std::vector<char> input_buffer(size);
    input.read(input_buffer.data(),input_buffer.size());

    return std::make_optional(input_buffer);
}

int main(int argc, const char* argv[]) {
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

        
    ELFIO::elfio reader;

    // Load existing 32-bit MIPS ELF file
    if (!reader.load(argv[2])) {
        std::cerr << "Failed to load ELF file\n";
        return 1;
    }

    // Check it's ELF32 and MIPS
    if (reader.get_class() != ELFIO::ELFCLASS32 || reader.get_machine() != ELFIO::EM_MIPS) {
        std::cerr << "Not a 32-bit MIPS ELF file\n";
        return 1;
    }

    auto last_segment = reader.segments[reader.segments.size()-1];
    ELFIO::Elf32_Addr new_segment_virtual_addr = align(
                    last_segment->get_virtual_address() + last_segment->get_memory_size(),ALIGN);

    if(command == "vaddr" && argc == 3){
        print_vaddr(new_segment_virtual_addr);
    }else if(command == "expand" && argc == 5){
        auto segment_data = load_file(argv[4]);
        if(!segment_data.has_value()){
            std::cout << "Could not open file " << argv[4] << std::endl;
            return 1;
        }

        auto flags = reader.segments[0]->get_flags();
        reader.segments[0]->set_flags(flags | ELFIO::PF_W);

        // Add a PT_LOAD segment
        ELFIO::segment* new_seg = reader.segments.add();
        new_seg->set_type(ELFIO::PT_LOAD);
        new_seg->set_flags(ELFIO::PF_R | ELFIO::PF_W | ELFIO::PF_X);
        new_seg->set_align(ALIGN);
        new_seg->set_virtual_address(new_segment_virtual_addr);
        new_seg->set_file_size(SEGMENT_SIZE);
        new_seg->set_memory_size(SEGMENT_SIZE);

        // Create new section
        ELFIO::section* new_sec = reader.sections.add(".mydata");
        new_sec->set_type(ELFIO::SHT_PROGBITS);
        new_sec->set_flags(ELFIO::SHF_ALLOC | ELFIO::SHF_WRITE | ELFIO::SHF_EXECINSTR);
        new_sec->set_addr_align(ALIGN);
        new_sec->set_address(new_segment_virtual_addr);

        new_sec->set_data(segment_data.value().data(), segment_data.value().size());

        // Connect section to segment
        new_seg->add_section_index(new_sec->get_index(), new_sec->get_addr_align());

        // Save to new file
        if (!reader.save(argv[3])) {
            std::cerr << "Failed to save ELF\n";
            return 1;
        }

        // We need to get the new offset of the .rodata.sceModuleInfo section, because on PSP the physical address of the
        // first segment is abused to contain the address of that section.
        ELFIO::Elf32_Off sceModuleInfo_offset = 0;
        for(auto& sec : reader.sections){
            if(sec->get_name() == ".rodata.sceModuleInfo"){
                sceModuleInfo_offset = sec->get_offset();
                break;
            }
        }

        if(sceModuleInfo_offset == 0){
            std::cout << "Could not find section .rodata.sceModuleInfo\n";
            return 1;
        }

        reader.segments[0]->set_physical_address(sceModuleInfo_offset);

        // Save again the file
        if (!reader.save(argv[3])) {
            std::cerr << "Failed to save ELF\n";
            return 1;
        }
        std::cout << "New segment added!\n";

    }else{
        std::cout << "Unknown command " << command << " or wrong arguments." << std::endl;
        return 1;
    }

    return 0;
}
