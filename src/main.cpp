#include <iostream>
#include <SDL.h>

#include <mat4x4.h>
#include <vector3.h>

#include "renderer.h"
#include "buffer.h"

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

	ID buff = alfodr::buffer::create(rend._bufferData, 3 * sizeof(alfar::Vector3), sizeof(alfar::Vector3));

	alfar::Vector3 tri[3] = {{-1, 0, 0}, {1, 0, 0}, {0,1,0}};

	alfodr::buffer::upload(rend._bufferData, buff, tri, 3 * sizeof(alfar::Vector3));

	ID constantBuffer = alfodr::buffer::create(rend._bufferData, 3 * sizeof(alfar::Matrix4x4), 0);

	alfar::Matrix4x4 model = alfar::mat4x4::identity();
	alfar::Matrix4x4 view = alfar::mat4x4::lookAt(alfar::vector3::create(0,0,-5.f), alfar::vector3::create(0,0,0), alfar::vector3::create(0,1,0));
	alfar::Matrix4x4 projection  = alfar::mat4x4::persp(60.0*3.14f/180.0f, 640.0f/480.0f, 0.001f, 100.0f);

	alfar::Matrix4x4 mat[3] = {model,view,projection};

	alfodr::buffer::upload(rend._bufferData, constantBuffer, mat, 3 * sizeof(alfar::Matrix4x4));


	alfodr::renderer::bindBuffer(rend, alfodr::VERTEXDATA, buff);
	alfodr::renderer::bindBuffer(rend, alfodr::CONSTANTDATA, constantBuffer);
	alfodr::renderer::draw(rend, 3);

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
