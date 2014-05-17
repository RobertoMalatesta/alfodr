#include "shaders.h"
#include "renderer.h"

using namespace alfodr;


alfar::Vector4 sampler2d::sample(Renderer& renderer, uint32 sampler, alfar::Vector4 uv)
{
	int x,y;
	Texture2D& tex = renderer._textures2D.lookup(renderer.samplers[sampler].texture);

	x = ((int)(uv.x * tex._width)) % tex._width;
	y = ((int)((1.0f-uv.y) * tex._height)) % tex._height;


	BGRA* pointer = (BGRA*)&renderer._bufferData._bufferMemory[renderer._bufferData._buffers.lookup(tex._buffer)._dataOffset];
	BGRA value = pointer[y * tex._width + x];

	return alfar::vector4::create((float)value.r / 0xFF, (float)value.g / 0xFF, (float)value.b / 0xFF, (float)value.a / 0xFF);
}