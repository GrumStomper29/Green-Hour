cmake_minimum_required(VERSION 3.28.0)

project(Green_Hour LANGUAGES CXX)

add_subdirectory(third_party)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES src/main.cpp)
add_executable(Green_Hour ${SOURCES})

# not needed, unless you are planning to manually pull stuff like stb_image.h
#target_include_directories(Green_Hour PRIVATE third_party)

target_link_libraries(Green_Hour PRIVATE
	SDL3::SDL3 
	VulkanMemoryAllocator 
	daxa 
	Vulkan-Headers
	EnTT)
