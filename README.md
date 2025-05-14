# psp_elf_expand
Simple tool to add a new segment to a PSP decrypted EBOOT.BIN for adding custom code and data to a PSP game.

First, you ask the virtual address where the new segment will be loaded in memory:

`psp_elf_expand vaddr <decrypted_eboot_file>`

Then, after you create a binary file `segment.bin` containing your custom code and data properly configured for the given virtual address, you can add it to the decrypted EBOOT as follows:

`psp_elf_expand expand <decrypted_eboot_file> <new_eboot_file> segment.bin`

# Compiling

On linux and macOS you just need a C++ compiler like g++, cmake and make (or ninja). On Windows you need the MSVC C++ compier and cmake.

On Linux and macOS, open a terminal, navigate to the project directory and run `cmake --preset release-makefiles` or `cmake --preset release-ninja` if you use ninja instead of make. Then compile with `cmake --build build`

On Windows, open a terminal, navigate to the project directory and run `cmake --preset msvc`. Then, compile with `cmake --build build --config Release`.
