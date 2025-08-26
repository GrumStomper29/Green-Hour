cmake_minimum_required(VERSION 4.1.0)

include(FetchContent)

project(Green_Hour)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(src)

FetchContent_Declare(
	SDL 
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG main)
FetchContent_MakeAvailable(SDL)

#add_subdirectory()
target_link_libraries(Green_Hour PRIVATE SDL3::SDL3)

#add_custom_command(
#	TARGET Green_Hour POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_if_different
#	$<TARGET_FILE:SDL3::SDL3>
#	$<TARGET_FILE_DIR:Green_Hour>
#)

message(STATUS "SDL3 DLL path: $<TARGET_FILE:SDL3::SDL3>")