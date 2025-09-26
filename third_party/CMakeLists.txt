include(FetchContent)

set(SDL_STATIC ON)
FetchContent_Declare(
    Vulkan-Headers 
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
    GIT_TAG        v1.4.325
)

#FetchContent_Declare(
#    VulkanMemoryAllocator 
#    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#    GIT_TAG        v3.3.0
#    FIND_PACKAGE_ARGS
#)

set(SDL_TESTS_LINK_SHARED off)
set(SDL_TEST_LIBRARY off)
FetchContent_Declare(
	sdl 
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG        release-3.2.20
)

set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG on)
set(DAXA_ENABLE_UTILS_MEM on)
set(DAXA_ENABLE_UTILS_TASK_GRAPH on)
FetchContent_Declare(
    daxa 
    GIT_REPOSITORY https://github.com/Ipotrick/Daxa 
    GIT_TAG        rework-build-system
#3.1
)

FetchContent_Declare(
	EnTT
	GIT_REPOSITORY https://github.com/skypjack/entt
	GIT_TAG        v3.15.0
)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm
	GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(
	sdl 
	Vulkan-Headers 
	# VulkanMemoryAllocator 
	daxa
	EnTT
	glm
)


