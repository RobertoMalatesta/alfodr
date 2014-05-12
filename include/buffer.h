#pragma once

#include <types.h>
#include "objectmanager.h"

#define DEFAULT_MAX_BUFFER 1024

namespace alfodr
{
	//Define a buffer given to the pipeline.
	//ATM 2 types :
	// - vertex : just all vertex data send with drawcall
	// - constant : info used in the vertex function.
	struct Buffer : ManagedObject
	{
		uint32 size;
		//used when a vertex buffer to define where vertex start & when it end.
		uint16 stride;

		//buffer are stocked contiguously in memory, following :
		//This is the offset starting at _gBufferMemory at which is this buffer data.
		uint32 _dataOffset;
	};


	namespace buffer
	{
		extern uint8* _gBufferMemory;
		extern ObjectManager<Buffer, DEFAULT_MAX_BUFFER> _gBuffers;

		void initSubsystem();

		//create a buffer allocating size.
		ID create(uint32 size);
		void upload(const Buffer& b, void* data, uint32 dataSize, uint16 stride = 0);
	}
}