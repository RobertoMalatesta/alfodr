#include <iostream>
#include <SDL.h>

#include <Vector3.h>

#include "renderer.h"

const int width = 640;
const int height = 480;

/*
 * Lesson 1: Hello World!
 */
int main(int argc, char** argv)
{
	//First we need to start up SDL, and make sure it went ok
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Now create a window with title "Hello World" at 100, 100 on the screen with w:640 h:480 and show it
	SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, width, height, SDL_WINDOW_SHOWN);
	//Make sure creating our window went ok
	if (win == nullptr){
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	//-----

	alfodr::Renderer rend;
	alfodr::renderer::initialize(rend, width, height);

	alfodr::renderer::rasterize(rend, alfar::vector3::create(10, 10, 0), alfar::vector3::create(10,100,0), alfar::vector3::create(100,100,0));

	//-----

	SDL_Surface* screen = SDL_GetWindowSurface(win);

	bool quit = false;
	SDL_Event e;

	while (!quit)
	{
		//Read any events that occured, for now we'll just quit if any event occurs
		while (SDL_PollEvent(&e))
		{
			//If user closes the window
			if (e.type == SDL_QUIT){
				quit = true;
			}
		}

		SDL_LockSurface(screen);
		memcpy(screen->pixels, rend._internalBuffer, width*height*4);
		SDL_UnlockSurface(screen);
		SDL_UpdateWindowSurface(win);
	}

	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
