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

	//Manage all buffer, used throught renderer usually.
	struct BufferManager
	{
		uint8* _bufferMemory;
		uint32 _bufferSize;
		ObjectManager<Buffer, DEFAULT_MAX_BUFFER> _buffers;
	};


	namespace buffer
	{
		void initManager(BufferManager& manager);

		//create a buffer allocating size.
		ID create(BufferManager& manager, uint32 size, uint16 stride = 0);
		void upload(BufferManager& manager, ID buffer, void* data, uint32 dataSize, uint32 offset = 0);
	}
}