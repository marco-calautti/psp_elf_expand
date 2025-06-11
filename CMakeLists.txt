cmake_minimum_required (VERSION 3.30)

project ("psp_elf_expand" VERSION 1.0)

# configure compiler
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(elfio REQUIRED)

# Include main project source files
include_directories (include)
FILE(GLOB_RECURSE SOURCES src/*.cpp)


# configure executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE elfio::elfio)
