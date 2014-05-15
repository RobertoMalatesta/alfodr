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

	typedef void(*pixFunc)(VertexOutput* input, void* data,	alfar::Vector4* output);


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
		INDEXDATA,
		CONSTANTDATA
	};

	struct Renderer
	{
		ARGB* _internalBuffer;
		int32 w,h;

		BufferManager _bufferData;

		//this will be send to the "vertex" function, splitting @ strides
		ID _vertexBufferBound;
		ID _indexBufferBound;
		ID _constantBufferBound;

		vertFunc boundVertexFunc;
		pixFunc boundPixFunc;
	};

	namespace renderer
	{
		void initialize(Renderer& rend, int w, int h);
		void rasterize(Renderer& rend, const VertexOutput vertex1, const VertexOutput vertex2, const VertexOutput vertex3);

		void bindBuffer(Renderer& rend, EBindTarget target, ID buffer);

		void draw(Renderer& rend, const uint32 primitiveCount);

		//-----------------------------------

		void clear(Renderer& rend, ARGB value);
	}
}