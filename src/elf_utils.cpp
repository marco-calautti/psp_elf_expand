#include <elf_utils.h>
#include <vector>

static uint32_t align(uint32_t value, uint32_t align){
    return (value + (align-1))/align*align;
}

static const uint32_t ALIGN = 0x40;

static uint32_t expand_or_vaddr(std::vector<uint8_t>& elf, const std::vector<uint8_t>& new_segment_data, bool vaddr_only){

    std::size_t elf_size = elf.size();

    elf_header_t* elf_header = reinterpret_cast<elf_header_t*>(elf.data());
    uint16_t num_segments = elf_header->e_phnum;

    elf_segment_header_t* p_elf_segments = 
        reinterpret_cast<elf_segment_header_t*>(elf.data() + elf_header->e_phoff);
    std::vector<elf_segment_header_t> elf_segments;
    for(std::size_t i = 0; i < elf_header->e_phnum; i++){
        elf_segment_header_t hdr = p_elf_segments[i];
        elf_segments.push_back(hdr);
    }
    
    elf_segments[0].p_flags = 0x07; // Make .text Read + Write + Execute

    elf_segment_header_t last_segment = elf_segments[num_segments-1];

    uint32_t new_segment_virtual_addr = align(
                    last_segment.p_vaddr + last_segment.p_memsz,ALIGN);

    if(vaddr_only){
        return new_segment_virtual_addr;
    }

    uint32_t new_header_offset = align(elf_size,0x10);
    elf_header->e_phnum = num_segments + 1;
    elf_header->e_phoff = new_header_offset;

    elf_segment_header_t new_segment = {0};

    std::size_t new_segment_size = new_segment_data.size();
    new_segment.p_type = 0x1; //PT_LOAD
    new_segment.p_offset = align(new_header_offset+sizeof(elf_segment_header_t)*(num_segments+1),0x10);
    new_segment.p_vaddr = new_segment_virtual_addr;
    new_segment.p_filesz = new_segment_size;
    new_segment.p_memsz = new_segment_size;
    new_segment.p_flags = 0x07; //Read + Write + Execute
    new_segment.p_align = ALIGN;

    // Alignment
    for(size_t i = 0; i < new_header_offset - elf_size; i++){
        elf.push_back(0);
    }

    for(auto segment : elf_segments){
        elf.insert(elf.end(),(uint8_t*)(&segment),(uint8_t*)(&segment+1));
    }
    elf.insert(elf.end(),(uint8_t*)&new_segment,(uint8_t*)(&new_segment+1));

    // Alignment
    std::size_t partial_new_elf_size = elf.size();
    for(size_t i = 0; i < new_segment.p_offset - partial_new_elf_size; i++){
        elf.push_back(0);
    }

    // Write new segment
    elf.insert(elf.end(),new_segment_data.begin(),new_segment_data.end());

    return new_segment_virtual_addr;
}

uint32_t elf_expand(std::vector<uint8_t>& elf, const std::vector<uint8_t>& new_segment_data){
    return expand_or_vaddr(elf,new_segment_data,false);
}

uint32_t elf_new_segment_vaddr(std::vector<uint8_t>& elf){
    return expand_or_vaddr(elf,{},true);
}

