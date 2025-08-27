include(FetchContent)

FetchContent_Declare(
    Vulkan-Headers 
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
    GIT_TAG        v1.4.325
)
FetchContent_Declare(
    VulkanMemoryAllocator 
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG v3.3.0
    FIND_PACKAGE_ARGS
)
FetchContent_Declare(
	sdl 
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG release-3.2.20
)
FetchContent_Declare(
    daxa 
    GIT_REPOSITORY https://github.com/Ipotrick/Daxa 
    GIT_TAG 3.1

)
FetchContent_MakeAvailable(sdl Vulkan-Headers VulkanMemoryAllocator daxa)


