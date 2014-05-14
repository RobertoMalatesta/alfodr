#pragma once

#include <types.h>
#include <vector3.h>

namespace alfodr
{
	//WARNING : Super hacky test function, remember TO FREE THE RETURNED DATA
	void meshFromFile(const char* file, uint32& nbVert, uint32& nbFace, alfar::Vector4* &outVerts, uint32* &outIndices, uint32& strides);
}