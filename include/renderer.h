#pragma once

#include <types.h>
#include <math_types.h>

#include <atomic>

#include "buffer.h"
#include "texture.h"
#include "shaders.h"

namespace alfodr
{
	struct Renderer;

	struct VertexOutput
	{
		alfar::Vector4 position;
		alfar::Vector4 interpolant1;
		alfar::Vector4 interpolant2;
		alfar::Vector4 interpolant3;
	};

	//this is the function type used by the vertex functions.
	//data should be cast to whatever your layout struct is. see in renderer.cpp for
	//the default impl using as simple input
	typedef void (*vertFunc)(void* vertData, void* constants, VertexOutput* output);

	typedef void(*pixFunc)(VertexOutput* input, Renderer* renderer,	alfar::Vector4* output);


	union BGRA
	{
		struct
		{
			uint8 b,g,r,a;
		};

		uint32 bgra;
	};

	enum EBindTarget
	{
		VERTEXDATA,
		INDEXDATA,
		CONSTANTDATA
	};

	struct Renderer
	{
		BGRA* _internalBuffer;
		uint16* _depthBuffer;
		int32 w,h;

		BufferManager _bufferData;
		ObjectManager<Texture2D, 512> _textures2D;

		//this will be send to the "vertex" function, splitting @ strides
		ID _vertexBufferBound;
		ID _indexBufferBound;
		ID _constantBufferBound;

		Sampler2D samplers[16];

		vertFunc boundVertexFunc;
		pixFunc boundPixFunc;


		std::atomic<int> _runningThreads;
	};

	namespace renderer
	{
		void initialize(Renderer& rend, int w, int h);
		void rasterize(Renderer& rend, const VertexOutput vertex1, const VertexOutput vertex2, const VertexOutput vertex3);

		void bindBuffer(Renderer& rend, EBindTarget target, ID buffer);

		void draw(Renderer& rend, const uint32 primitiveCount);

		//----------------------------------

		ID createTexture(Renderer& rend, uint32 width, uint32 height, TextureFormat format, void* data);

		void bindTexture(Renderer& rend, const uint16 sampler, ID texture);

		//-----------------------------------

		void clear(Renderer& rend, BGRA value);
	}
}