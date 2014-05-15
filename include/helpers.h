#pragma once

#include <types.h>
#include <vector3.h>

namespace alfodr
{
	struct SimpleVertex
	{
		alfar::Vector4 pos;
		alfar::Vector4 normal;
		alfar::Vector4 uv;
	};

	//WARNING : Super hacky test function, remember TO FREE THE RETURNED DATA
	void meshFromFile(const char* file, uint32& nbVert, uint32& nbFace, SimpleVertex* &outVerts, uint32* &outIndices);
}