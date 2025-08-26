#include <iostream>

// Do I need this?
#define SDL_MAIN_HANDLED
#include "SDL3/SDL.h"

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "SDL couldn't initialize video.\n";
	}

	auto window = SDL_CreateWindow("Green Hour", 640, 480, 0);

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

	

	return 0;
}