#pragma once

#include <types.h>
#include <math_types.h>

namespace alfodr
{
	union ARGB
	{
		struct
		{
			int8 b,g,r,a;
		};

		int32 argb;
	};

	struct Renderer
	{
		ARGB* _internalBuffer;
		int32 w,h;
	};

	namespace renderer
	{
		void initialize(Renderer& rend, int w, int h);
		void rasterize(Renderer& rend, const alfar::Vector3 v1, const alfar::Vector3 v2, const alfar::Vector3 v3);
	}
}