#include "buffer.h"
#include <memory>

using namespace alfodr;

/*
* The buffer is organize as follow :
* size - data.
* 32bit are appended to each buffer to stock it's size. A negative value (highest bit to 1) 
* mean that the space behind is free (of said size). 0 mark the end of buffer.
* that way we can minimize fragmentation
*/
void buffer::initManager(BufferManager& manager)
{
	manager._bufferSize = 1024*1024*256;
	manager._bufferMemory = (uint8*)malloc(manager._bufferSize);
	memset(manager._bufferMemory, 0, manager._bufferSize);
}

//----------------------------

ID buffer::create(BufferManager& manager, uint32 size, uint16 stride)
{
	Buffer& b = manager._buffers.addAndGet();

	//find next free space fiting it in the raw memory
	uint32 offset = 0;

	while( *((int32*)(manager._bufferMemory + offset)) > 0 && offset < manager._bufferSize)
	{//while we don't reach a free space or the end (full buffer)
		offset += *((int32*)(manager._bufferMemory + offset)) + 4;
	}

	while(offset + size >= manager._bufferSize)
	{//expand the buffer until we can fit the wanted size
		uint32 newsize = manager._bufferSize + 1024*1024 * 128;
		uint8* newmem = (uint8*)malloc(newsize);
		memcpy(newmem, manager._bufferMemory, manager._bufferSize);
		memset(newmem + manager._bufferSize, 0, newsize - manager._bufferSize);

		free(manager._bufferMemory);
		
		manager._bufferMemory = newmem;
		manager._bufferSize = newsize;
	}

	*((uint32*)(manager._bufferMemory + offset)) = size;
	

	b.size = size;
	b.stride = stride;
	b._dataOffset = offset + 4;

	return b.id;
}

void buffer::upload(BufferManager& manager, ID buffer, void* data, uint32 dataSize, uint32 offset)
{
	Buffer& b = manager._buffers.lookup(buffer);

	_ASSERT(b.size >= dataSize + offset);

	memcpy(manager._bufferMemory + b._dataOffset + offset, data, dataSize);
}