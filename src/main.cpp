#include <iostream>
#include <SDL.h>
#include <SDL_image.h>


#include <mat4x4.h>
#include <vector3.h>
#include <quaternion.h>

#include "renderer.h"
#include "buffer.h"
#include "helpers.h"

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

	SDL_DisplayMode mode;
	SDL_GetWindowDisplayMode(win, &mode);

	mode.format = SDL_PIXELFORMAT_RGBA8888;

	SDL_SetWindowDisplayMode(win, &mode);

	//-----

	alfodr::SimpleVertex* verts;
	uint32* indices;
	uint32 vertNb, indicesNb;
	//alfodr::meshFromFile("test/capsule.obj", vertNb, indicesNb, verts, indices);
	alfodr::meshFromMemap("test/Trex.mesh", vertNb, indicesNb, verts, indices);

	alfodr::BGRA black;
	black.bgra = 0x0;
	black.g = 200;
	black.b = 255;

	alfar::Quaternion rot = alfar::quaternion::identity();
	
	alfodr::Renderer rend;
	alfodr::renderer::initialize(rend, width, height);

	ID buff = alfodr::buffer::create(rend._bufferData, vertNb * sizeof(alfodr::SimpleVertex), sizeof(alfodr::SimpleVertex));
	ID idxBuff = alfodr::buffer::create(rend._bufferData, indicesNb * sizeof(uint32), sizeof(uint32));

	//alfar::Vector3 tri[4] = {{-1, 0, 0}, {1, 0, 0}, {0,1,0}, {0, -1, 0}};
	//alfodr::buffer::upload(rend._bufferData, buff, tri, 4 * sizeof(alfar::Vector3));
	alfodr::buffer::upload(rend._bufferData, buff, verts, vertNb * sizeof(alfodr::SimpleVertex));

	//uint32 idx[6] = {0,1,2,0,3,1};
	alfodr::buffer::upload(rend._bufferData, idxBuff, indices, indicesNb * sizeof(uint32));

	free(indices);
	free(verts);

	ID constantBuffer = alfodr::buffer::create(rend._bufferData, 3 * sizeof(alfar::Matrix4x4), 0);

	alfar::Matrix4x4 model = alfar::quaternion::toMat4x4(rot);
	alfar::Matrix4x4 view = alfar::mat4x4::lookAt(alfar::vector3::create(0, 7.f,-15.0f), alfar::vector3::create(0,4.f,0), alfar::vector3::create(0,1,0));
	alfar::Matrix4x4 projection  = alfar::mat4x4::persp(60.0f*3.14f/180.0f, 640.0f/480.0f, 0.1f, 1000.0f);

	alfar::Matrix4x4 mat[3] = {model,view,projection};

	alfodr::buffer::upload(rend._bufferData, constantBuffer, mat, 3 * sizeof(alfar::Matrix4x4));

	alfodr::renderer::bindBuffer(rend, alfodr::VERTEXDATA, buff);
	alfodr::renderer::bindBuffer(rend, alfodr::INDEXDATA, idxBuff);
	alfodr::renderer::bindBuffer(rend, alfodr::CONSTANTDATA, constantBuffer);


	
	SDL_Surface* bmp = IMG_Load("test/trextex.png");
	SDL_Surface* conv = SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_ARGB8888, 0);

	SDL_FreeSurface(bmp);
	bmp = conv;

	SDL_LockSurface(bmp);
	ID texture = alfodr::renderer::createTexture(rend, bmp->w, bmp->h, alfodr::TexFormat_BGRA, bmp->pixels);
	alfodr::renderer::bindTexture(rend, 0, texture);
	SDL_UnlockSurface(bmp);
	SDL_FreeSurface(bmp);


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
		alfodr::renderer::draw(rend, indicesNb/3);

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
			std::cout<<fpsNumber<<" FPS ; "<<1000.0f/fpsNumber<<" ms"<<std::endl; 

			fpsNumber = 0.0f;
			lastFpsDisplay = 0;
		}
	}

	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
