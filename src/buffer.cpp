#include "buffer.h"
#include <memory>

using namespace alfodr;


ObjectManager<Buffer, DEFAULT_MAX_BUFFER> buffer::_gBuffers;
uint8* buffer::_gBufferMemory;

uint32 _gBufferMemorySize = 1024*1024;


void buffer::initSubsystem()
{
	_gBufferMemory = (uint8*)malloc(_gBufferMemorySize);

	//when allocating space in the buffer, we append the size before on 31bits. 
	//highest bit tell if it's free (0) or allocated (1)
	memset(_gBufferMemory, 0, _gBufferMemorySize);
}

//----------------------------

ID buffer::create(uint32 size)
{
	Buffer& b = _gBuffers.addAndGet();

	b.size = 32;

	//find next free space fiting it in the raw memory
	uint32 offset = 0;

	while( (*((uint32*)(_gBufferMemory + offset)) & 0x80000000) != 0 && offset < _gBufferMemorySize)
	{//while we don't reach a free space or the end (full buffer)
		uint32 info = *((uint32*)_gBufferMemory+offset);
		info = (info << 1) >> 1; //remove the "free" bit

		offset += info;
	}

	while(offset + size >= _gBufferMemorySize)
	{
		uint32 newsize = _gBufferMemorySize + 1024*1024;
		uint8* newmem = (uint8*)malloc(newsize);
		memcpy(newmem, _gBufferMemory, _gBufferMemorySize);
		memset(newmem + _gBufferMemorySize, 0, newsize - _gBufferMemorySize);

		free(_gBufferMemory);
		
		_gBufferMemory = newmem;
		_gBufferMemorySize = newsize;
	}

	_gBufferMemory[offset] = size;

}