#pragma once

#include <types.h>
#include <vector4.h>
#include "buffer.h"

/*
* texture are just a buffer with added width/height/format information.
*/
namespace alfodr
{
	enum TextureFormat
	{
		TexFormat_BGRA
	};


	struct Texture2D : ManagedObject
	{
		uint32 _width;
		uint32 _height;
		ID _buffer;
	};
}