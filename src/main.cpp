#include <iostream>
#include <SDL.h>

#include <mat4x4.h>
#include <vector3.h>
#include <quaternion.h>

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

	alfodr::ARGB black;
	black.argb = 0x0;

	alfar::Quaternion rot = alfar::quaternion::identity();
	
	alfodr::Renderer rend;
	alfodr::renderer::initialize(rend, width, height);

	ID buff = alfodr::buffer::create(rend._bufferData, 4 * sizeof(alfar::Vector3), sizeof(alfar::Vector3));
	ID idxBuff = alfodr::buffer::create(rend._bufferData, 6 * sizeof(uint32), sizeof(uint32));

	alfar::Vector3 tri[4] = {{-1, 0, 0}, {1, 0, 0}, {0,1,0}, {0, -1, 0}};
	alfodr::buffer::upload(rend._bufferData, buff, tri, 4 * sizeof(alfar::Vector3));

	uint32 idx[6] = {0,1,2,0,3,1};
	alfodr::buffer::upload(rend._bufferData, idxBuff, idx, 6 * sizeof(uint32));

	ID constantBuffer = alfodr::buffer::create(rend._bufferData, 3 * sizeof(alfar::Matrix4x4), 0);

	alfar::Matrix4x4 model = alfar::quaternion::toMat4x4(rot);
	alfar::Matrix4x4 view = alfar::mat4x4::lookAt(alfar::vector3::create(0, 0,-8.f), alfar::vector3::create(0,0.f,0), alfar::vector3::create(0,1,0));
	alfar::Matrix4x4 projection  = alfar::mat4x4::persp(60.0f*3.14f/180.0f, 640.0f/480.0f, 0.001f, 100.0f);

	alfar::Matrix4x4 mat[3] = {model,view,projection};

	alfodr::buffer::upload(rend._bufferData, constantBuffer, mat, 3 * sizeof(alfar::Matrix4x4));

	alfodr::renderer::bindBuffer(rend, alfodr::VERTEXDATA, buff);
	alfodr::renderer::bindBuffer(rend, alfodr::INDEXDATA, idxBuff);
	alfodr::renderer::bindBuffer(rend, alfodr::CONSTANTDATA, constantBuffer);

	//-----

	SDL_Surface* screen = SDL_GetWindowSurface(win);

	bool quit = false;
	SDL_Event e;

	uint32 startTime = SDL_GetTicks();
	uint32 lastFrame = startTime;
	uint32 lastFpsDisplay = 0;
	uint32 fpsNumber = 0;

	float deltaTime = 0.0f;

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

		rot = alfar::quaternion::mul(alfar::quaternion::axisAngle(alfar::vector3::create(0,1,0), (25.0f * 3.14f / 180.0f) * deltaTime), rot);
		model  = alfar::quaternion::toMat4x4(rot);
		alfodr::buffer::upload(rend._bufferData, constantBuffer, &model, sizeof(alfar::Matrix4x4));

		alfodr::renderer::clear(rend, black);
		alfodr::renderer::draw(rend, 2);

		SDL_LockSurface(screen);
		memcpy(screen->pixels, rend._internalBuffer, width*height*4);
		SDL_UnlockSurface(screen);
		SDL_UpdateWindowSurface(win);

		int time = SDL_GetTicks() - startTime;
		int delta = time - lastFrame;

		lastFpsDisplay += delta;
		fpsNumber += 1;

		lastFrame = time;

		deltaTime = delta / 1000.0f;

		if(lastFpsDisplay >= 1000)
		{
			system("cls");
			std::cout<<fpsNumber<<" FPS ; "<<1.0f/fpsNumber<<" ms"<<std::endl; 

			fpsNumber = 0.0f;
			lastFpsDisplay = 0;
		}
	}

	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
