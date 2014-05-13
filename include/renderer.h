#pragma once

#include <types.h>
#include <math_types.h>

#include "buffer.h"

namespace alfodr
{

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


	union ARGB
	{
		struct
		{
			int8 b,g,r,a;
		};

		int32 argb;
	};

	enum EBindTarget
	{
		VERTEXDATA,
		CONSTANTDATA
	};

	struct Renderer
	{
		ARGB* _internalBuffer;
		int32 w,h;

		BufferManager _bufferData;

		//this will be send to the "vertex" function, splitting @ strides
		ID _vertexBufferBound;
		ID _constantBufferBound;

		vertFunc boundVertexFunc;
	};

	namespace renderer
	{
		void initialize(Renderer& rend, int w, int h);
		void rasterize(Renderer& rend, const alfar::Vector4 v1, const alfar::Vector4 v2, const alfar::Vector4 v3);

		void bindBuffer(Renderer& rend, EBindTarget target, ID buffer);

		void draw(Renderer& rend, const uint32 vertexCount);

		//-----------------------------------
	}
}