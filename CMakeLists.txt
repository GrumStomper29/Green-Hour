cmake_minimum_required(VERSION 4.1.0)

include(FetchContent)

project(Green_Hour)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES src/main.cpp third_party/vma/vk_mem_alloc.h)
add_executable(Green_Hour ${SOURCES})

target_include_directories(Green_Hour PRIVATE third_party)


FetchContent_Declare(
	SDL 
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG main)
FetchContent_MakeAvailable(SDL)
target_link_libraries(Green_Hour PRIVATE SDL3::SDL3)

FetchContent_Declare(
	Daxa
	GIT_REPOSITORY https://github.com/Ipotrick/Daxa
	GIT_TAG master)
FetchContent_MakeAvailable(Daxa)