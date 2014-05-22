#include "renderer.h"
#include "helpers.h"
#include "shaders.h"
#include <functions.h>
#include <memory>

#include <vector>

#include <mat4x4.h>
#include <vector4.h>

using namespace alfodr;
using namespace alfar;

#define MULTITHREADED_RENDER 1


//--------- INTERNAL FUNCTION

void fixFunctionVertex(void* vertData, void* constants, VertexOutput* output)
{
	alfar::Matrix4x4 model = *(((alfar::Matrix4x4*)constants));
	alfar::Matrix4x4 view = *(((alfar::Matrix4x4*)constants) + 1);
	alfar::Matrix4x4 projection = *(((alfar::Matrix4x4*)constants) + 2);

	SimpleVertex v = *((SimpleVertex*)vertData);
	//alfar::Vector4 v = *((alfar::Vector4*)vertData);


	alfar::Vector4 pos = alfar::vector4::mul(model, v.pos);
	pos = alfar::vector4::mul(view, pos);
	pos = alfar::vector4::mul(projection, pos);

	output->position = pos;
	output->interpolant1 = v.uv;
	output->interpolant2 = alfar::vector4::mul(model, v.normal);
}

void FixFunctionPixel(VertexOutput* input, Renderer* rend, Vector4* output)
{
	alfar::Vector4 normal = input->interpolant2;

	float lum = std::max(0.0f, alfar::vector4::dot(normal, alfar::vector4::normalize(alfar::vector4::create(0.5f, 0.5f, 0, 0))));
	
	*output = alfar::vector4::mul(sampler2d::sample(*rend, 0, input->interpolant1), lum*2.0f);
}


void thread_handleInBlock(Renderer& rend, int x, int y, Vector3 v1, Vector3 v2, Vector3 v3, VertexOutput* vs)
{
	VertexOutput v;
	Vector4 output;
	Vector3 bar = vector3::barycentric(v1,v2,v3, vector3::create(x, y, 0));

	v.position = vector4::interpolatedFromBarycentric(vector4::create(v1), vector4::create(v2), vector4::create(v3), bar);

	if(v.position.z < 0.0f || v.position.z > 1.0f || v.position.z * ((uint16)-1) > rend._depthBuffer[y*rend.w + x])
		return;

	v.interpolant1 = vector4::interpolatedFromBarycentric(vs[0].interpolant1, vs[1].interpolant1, vs[2].interpolant1, bar);
	v.interpolant2 = vector4::interpolatedFromBarycentric(vs[0].interpolant2, vs[1].interpolant2, vs[2].interpolant2, bar);
	v.interpolant3 = vector4::interpolatedFromBarycentric(vs[0].interpolant3, vs[1].interpolant3, vs[2].interpolant3, bar);

	rend.boundPixFunc(&v, &rend, &output);

	output = vector4::saturate(output);

	rend._internalBuffer[y * rend.w + x].r = output.x * 0xFF;
	rend._internalBuffer[y * rend.w + x].g = output.y * 0xFF;
	rend._internalBuffer[y * rend.w + x].b = output.z * 0xFF;

	rend._depthBuffer[y * rend.w + x] = v.position.z * ((uint16)-1);
}

void thread_handleOnEdge(Renderer& rend, int minX, int minY, int q, Vector3 v1, Vector3 v2, Vector3 v3, VertexOutput* vs)
{

}

void thread_Draw(thread_DrawInfo* info, thread_JobInfo* id)
{
	do
	{
		if(id->doingJob)
		{
			for(int i = id->offset; i < id->offset + id->count; ++i)
			{
				uint32 firstVert =	*(((uint32*)info->idxData) + (i * 3));
				uint32 secondVert = *(((uint32*)info->idxData) + (i * 3 + 1));
				uint32 thirdVert =	*(((uint32*)info->idxData) + (i * 3 + 2));

				info->rend->boundVertexFunc((info->vertData + info->stride*firstVert) , info->constData, info->outputs + 3*i);
				info->rend->boundVertexFunc((info->vertData + info->stride*secondVert), info->constData, info->outputs + 3*i + 1 );
				info->rend->boundVertexFunc((info->vertData + info->stride*thirdVert) , info->constData, info->outputs + 3*i +2 );
			}

			id->doingJob.store(false);
		}

		_sleep(0);
	}
	while(id->alive);
}

//---------------------------

void renderer::initialize(Renderer& rend, int w, int h)
{
	rend.w = w;
	rend.h = h;
	rend._internalBuffer = (BGRA*)malloc(w*h*sizeof(BGRA));
	rend._depthBuffer = (uint16*)malloc(w*h*sizeof(uint16));

	memset(rend._internalBuffer, 0, w*h*sizeof(BGRA));
	memset(rend._internalBuffer, 0, w*h*sizeof(uint16));

	rend.boundVertexFunc = fixFunctionVertex;
	rend.boundPixFunc = FixFunctionPixel;

#if MULTITHREADED_RENDER
	//-- launch k thread
	for(int i = 0; i < Renderer::kNbThread; ++i)
	{
		rend.threadJobInfo[i].alive.store(true);
		rend.threadJobInfo[i].doingJob.store(false);

		rend.threads[i] = std::thread(thread_Draw, &rend.currentDrawInfo, &rend.threadJobInfo[i]);
	}
#endif

	buffer::initManager(rend._bufferData);
}

void renderer::bindBuffer(Renderer& rend, EBindTarget target, ID buffer)
{
	switch(target)
	{
	case VERTEXDATA:
		rend._vertexBufferBound = buffer;
		break;
	case INDEXDATA:
		rend._indexBufferBound = buffer;
		break;
	case CONSTANTDATA:
		rend._constantBufferBound = buffer;
		break;
	default:
		break;
	};
}

void renderer::clear(Renderer& rend, BGRA value)
{
	for(int i = 0; i < rend.w*rend.h; ++i)
	{
		rend._internalBuffer[i] = value;
		rend._depthBuffer[i] = -1;
	}
}

void renderer::rasterize(Renderer& rend, const VertexOutput vertex1, const VertexOutput vertex2, const VertexOutput vertex3)
{
	VertexOutput vs[3] = {vertex1, vertex2, vertex3};

	Vector3 v1 = {vertex1.position.x/vertex1.position.w, vertex1.position.y/vertex1.position.w, vertex1.position.z/vertex1.position.w};
	Vector3 v2 = {vertex2.position.x/vertex2.position.w, vertex2.position.y/vertex2.position.w, vertex2.position.z/vertex2.position.w};
	Vector3 v3 = {vertex3.position.x/vertex3.position.w, vertex3.position.y/vertex3.position.w, vertex3.position.z/vertex3.position.w};

	v1.x = (v1.x + 0.5f) * rend.w;
	v2.x = (v2.x + 0.5f) * rend.w;
	v3.x = (v3.x + 0.5f) * rend.w;

	v1.y = rend.h - (v1.y + 0.5f) * rend.h;
	v2.y = rend.h - (v2.y + 0.5f) * rend.h;
	v3.y = rend.h - (v3.y + 0.5f) * rend.h;

	 // 28.4 fixed-point coordinates
	const int Y1 = iround(16.0f * v1.y);
    const int Y2 = iround(16.0f * v2.y);
    const int Y3 = iround(16.0f * v3.y);

    const int X1 = iround(16.0f * v1.x);
    const int X2 = iround(16.0f * v2.x);
    const int X3 = iround(16.0f * v3.x);

    // Deltas
    const int DX12 = X1 - X2;
    const int DX23 = X2 - X3;
    const int DX31 = X3 - X1;

    const int DY12 = Y1 - Y2;
    const int DY23 = Y2 - Y3;
    const int DY31 = Y3 - Y1;

    // Fixed-point deltas
    const int FDX12 = DX12 << 4;
    const int FDX23 = DX23 << 4;
    const int FDX31 = DX31 << 4;

    const int FDY12 = DY12 << 4;
    const int FDY23 = DY23 << 4;
    const int FDY31 = DY31 << 4;

    // Bounding rectangle
    int minx = (min3(X1, X2, X3) + 0xF) >> 4;
    int maxx = (max3(X1, X2, X3) + 0xF) >> 4;
    int miny = (min3(Y1, Y2, Y3) + 0xF) >> 4;
    int maxy = (max3(Y1, Y2, Y3) + 0xF) >> 4;

	if(minx >= rend.w || miny >= rend.h || maxx < 0 || maxy < 0)
		return; //rect outside of screen

	minx = (minx < 0 ? 0 : minx);
	miny = (miny < 0 ? 0 : miny);
	maxx = (maxx >= rend.w ? rend.w - 1 : maxx);
	maxy = (maxy >= rend.h ? rend.h - 1 : maxy);

    // Block size, standard 8x8 (must be power of two)
    const int q = 8;

    // Start in corner of 8x8 block
    minx &= ~(q - 1);
    miny &= ~(q - 1);

    // Half-edge constants
    int C1 = DY12 * X1 - DX12 * Y1;
    int C2 = DY23 * X2 - DX23 * Y2;
    int C3 = DY31 * X3 - DX31 * Y3;

    // Correct for fill convention
    if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
    if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
    if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

    // Loop through blocks
    for(int y = miny; y < maxy; y += q)
    {
        for(int x = minx; x < maxx; x += q)
        {
            // Corners of block
            int x0 = x << 4;
            int x1 = (x + q - 1) << 4;
            int y0 = y << 4;
            int y1 = (y + q - 1) << 4;

            // Evaluate half-space functions
            bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
            bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
            bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
            bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
            int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
    
            bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
            bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
            bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
            bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
            int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
    
            bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
            bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
            bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
            bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
            int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

            // Skip block when outside an edge
            if(a == 0x0 || b == 0x0 || c == 0x0) continue;

            // Accept whole block when totally covered
            if(a == 0xF && b == 0xF && c == 0xF)
            {
				for(int iy = y; iy < y + q; iy++)
				{
					for(int ix = x; ix < x + q; ix++)
					{
						thread_handleInBlock(rend, ix, iy, v1,v2,v3, vs);
					}
				}
            }
            else
            {
                int CY1 = C1 + DX12 * y0 - DY12 * x0;
                int CY2 = C2 + DX23 * y0 - DY23 * x0;
                int CY3 = C3 + DX31 * y0 - DY31 * x0;

                for(int iy = y; iy < y + q; iy++)
                {
                    int CX1 = CY1;
                    int CX2 = CY2;
                    int CX3 = CY3;

                    for(int ix = x; ix < x + q; ix++)
                    {
                        if(CX1 > 0 && CX2 > 0 && CX3 > 0)
                        {
							thread_handleInBlock(rend, ix, iy, v1,v2,v3, vs);
                        }

                        CX1 -= FDY12;
                        CX2 -= FDY23;
                        CX3 -= FDY31;
                    }

                    CY1 += FDX12;
                    CY2 += FDX23;
                    CY3 += FDX31;
                }
            }
        }
    }
}

void renderer::draw(Renderer& rend, const uint32 primitiveCount)
{
	VertexOutput* outputs = (VertexOutput*)malloc(primitiveCount * 3 * sizeof(VertexOutput));
	memset(outputs, 0, primitiveCount * 3 * sizeof(VertexOutput));

	Buffer& b = rend._bufferData._buffers.lookup(rend._vertexBufferBound);
	Buffer& idxBuffer = rend._bufferData._buffers.lookup(rend._indexBufferBound);

	uint8* constantData = NULL;
	if(rend._constantBufferBound != 0)
	{
		Buffer& cdata = rend._bufferData._buffers.lookup(rend._constantBufferBound);
		constantData = rend._bufferData._bufferMemory + cdata._dataOffset;
	}

	thread_DrawInfo info;
	info.constData = constantData;
	info.idxData = rend._bufferData._bufferMemory + idxBuffer._dataOffset;
	info.outputs = outputs;
	info.vertData = rend._bufferData._bufferMemory + (b._dataOffset);
	info.stride = b.stride;
	info.rend = &rend;

	rend.currentDrawInfo = info;
	
#if MULTITHREADED_RENDER 

	const uint32 nbPerSlices = 7000;
	int nbSlice = 1 + primitiveCount / nbPerSlices;
	int sliceLaunched = 0;
	do
	{
		for(uint32 i = 0; i < Renderer::kNbThread; ++i)
		{
			if(!rend.threadJobInfo[i].doingJob)
			{
				uint32 start = sliceLaunched * nbPerSlices;
				uint32 count = std::min(primitiveCount - start, nbPerSlices);
				rend.threadJobInfo[i].count = count;
				rend.threadJobInfo[i].offset = start;
				rend.threadJobInfo[i].doingJob.store(true);

				sliceLaunched += 1;
				break;
			}
		}
	}
	while(sliceLaunched < nbSlice);

	bool allfinished = false;
	
	while(!allfinished)
	{
		allfinished = true;
		for(uint32 i = 0; i <  Renderer::kNbThread; ++i)
		{
			if(rend.threadJobInfo[i].doingJob)
			{
				allfinished = false;
			}
		}
	}
#else
	thread_JobInfo infothread;
	infothread.alive.store(false); // exit after one iteration
	infothread.doingJob.store(true);
	infothread.count = primitiveCount;
	infothread.offset = 0;

	thread_Draw(&rend.currentDrawInfo, &infothread);
#endif
	

	for(int i = 0; i < primitiveCount; ++i)
	{
		//renderer::rasterize(rend, outputs[i*3], outputs[i*3+1], outputs[i*3+2]);
	}


	free(outputs);
}

//=============================================================

ID renderer::createTexture(Renderer& rend, uint32 width, uint32 height, TextureFormat format, void* data)
{
	uint16 stride = 0;

	switch(format)
	{
	case TexFormat_BGRA:
		stride = sizeof(BGRA);
		break;
	default:
		return 0;//unknown format, create nothing
	}

	Texture2D& tex = rend._textures2D.addAndGet();

	tex._buffer = buffer::create(rend._bufferData, width*height*stride, stride);
	buffer::upload(rend._bufferData, tex._buffer, data, width*height*stride);

	tex._width = width;
	tex._height = height;

	return tex.id;
}

void renderer::bindTexture(Renderer& rend, const uint16 sampler, ID texture)
{
	rend.samplers[sampler].texture = texture;
}