#include "SDL3/SDL.h"

//#include "vma/vk_mem_alloc.h"

#include <iostream>

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "SDL couldn't initialize video.\n";
	}

	SDL_WindowFlags windowFlags{ SDL_WINDOW_VULKAN };
	auto window = SDL_CreateWindow("Green Hour", 640, 480, windowFlags);

	bool windowShouldClose{ false };

	while (!windowShouldClose)
	{
		SDL_Event e{};
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				windowShouldClose = true;
			}
		}
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}