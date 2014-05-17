#pragma once

#include "objectmanager.h"
#include <math_types.h>

namespace alfodr
{
	struct Renderer;

	struct Sampler2D
	{
		ID texture;
	};

	namespace sampler2d
	{
		alfar::Vector4 sample(Renderer& renderer, uint32 sampler, alfar::Vector4 uv);
	}
}