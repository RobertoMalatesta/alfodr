#include "renderer.h"
#include <functions.h>
#include <memory>

#include <thread>

#include <mat4x4.h>
#include <vector4.h>

using namespace alfodr;
using namespace alfar;


//--------- INTERNAL FUNCTION

void fixFunctionVertex(void* vertData, void* constants, VertexOutput* output)
{
	alfar::Vector4 v = alfar::vector4::create(*((alfar::Vector3*)vertData));

	alfar::Matrix4x4 model = *(((alfar::Matrix4x4*)constants));
	alfar::Matrix4x4 view = *(((alfar::Matrix4x4*)constants) + 1);
	alfar::Matrix4x4 projection = *(((alfar::Matrix4x4*)constants) + 2);

	v = alfar::vector4::mul(model, v);
	v = alfar::vector4::mul(view, v);
	v = alfar::vector4::mul(projection, v);

	output->position = v;
}

//---------------------------

void renderer::initialize(Renderer& rend, int w, int h)
{
	rend.w = w;
	rend.h = h;
	rend._internalBuffer = (ARGB*)malloc(w*h*sizeof(ARGB));

	memset(rend._internalBuffer, 0, w*h*sizeof(ARGB));

	rend.boundVertexFunc = fixFunctionVertex;

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

void renderer::clear(Renderer& rend, ARGB value)
{
	for(int i = 0; i < rend.w*rend.h; ++i)
	{
		rend._internalBuffer[i] = value;
	}
}

void renderer::rasterize(Renderer& rend, const VertexOutput vertex1, const VertexOutput vertex2, const VertexOutput vertex3)
{

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
                for(int iy = 0; iy < q; iy++)
                {
                    for(int ix = x; ix < x + q; ix++)
                    {
                        //buffer[ix] = 0x00007F00; // Green
						rend._internalBuffer[(miny + y + iy) * rend.w + ix].b = 0xFF;
                    }

                    //(char*&)buffer += stride;
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
                            //buffer[ix] = 0x0000007F;

							rend._internalBuffer[(miny + iy) * rend.w + ix].r = 0xff;
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

void renderer::draw(Renderer& rend, const uint32 vertexCount)
{
	VertexOutput* outputs = (VertexOutput*)malloc(vertexCount * sizeof(VertexOutput));
	Buffer& b = rend._bufferData._buffers.lookup(rend._vertexBufferBound);

	void* constantData = NULL;
	if(rend._constantBufferBound != 0)
	{
		Buffer& cdata = rend._bufferData._buffers.lookup(rend._constantBufferBound);
		constantData = rend._bufferData._bufferMemory + cdata._dataOffset;
	}

	for(int i = 0; i < vertexCount; i+=3)
	{
		std::thread first(rend.boundVertexFunc, rend._bufferData._bufferMemory + (b._dataOffset + b.stride*i), constantData, outputs + 3*i);
		std::thread second(rend.boundVertexFunc, rend._bufferData._bufferMemory + (b._dataOffset + b.stride*(i+1)), constantData, outputs + 3*i + 1 );
		std::thread third(rend.boundVertexFunc, rend._bufferData._bufferMemory + (b._dataOffset + b.stride*(i+2)), constantData, outputs + 3*i +2 );

		first.join();
		second.join();
		third.join();

		renderer::rasterize(rend, outputs[i], outputs[i+1], outputs[i+2]);
	}


	free(outputs);
}