#include "helpers.h"
#include "renderer.h"

#include <vector4.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace alfodr;

void alfodr::meshFromFile(const char* file, uint32& nbVert, uint32& nbFace, SimpleVertex* &outVerts, uint32* &outIndices)
{
	std::ifstream myfile (file);

	if (myfile.is_open())
	{
		std::string line;
		std::vector<alfar::Vector4> verts, uvs, normals;
		std::vector<int> faces;

		std::string c = "#";
		while (myfile>>c)
		{
			if(c == "v")
			{
					float x,y,z;
					myfile>>x>>y>>z;

					verts.push_back(alfar::vector4::create(x,y,z,1));
			}
			else if(c == "vt")
			{
				float u,v;
				myfile>>u>>v;

				uvs.push_back(alfar::vector4::create(u,v,0,0));
			}
			else if(c == "vn")
			{
				float x,y,z;
				myfile>>x>>y>>z;

				normals.push_back(alfar::vector4::create(x,y,z,0));
			}
			else if(c == "f")
			{
				uint32 v1,v2,v3;
				myfile>>v1>>c>>v2>>c>>v3>>c;

				faces.push_back(v1-1);
				faces.push_back(v3-1);
				faces.push_back(v2-1);
			}
			else
			{
				std::getline(myfile, line);
			}
		}

		nbVert = verts.size();
		nbFace = faces.size();

		outVerts = (SimpleVertex*)malloc(nbVert * sizeof(SimpleVertex));
		outIndices = (uint32*)malloc(nbFace * sizeof(uint32));

		for(int i = 0; i < nbVert; ++i)
		{
			outVerts[i].pos = verts[i];

			if(uvs.size() > i)
				outVerts[i].uv = uvs[i];
			else
				outVerts[i].uv = alfar::vector4::create(0,0,0,0);

			if(normals.size() > i)
				outVerts[i].normal = normals[i];
			else
				outVerts[i].normal = alfar::vector4::create(0,0,0,0);
		}

		//memcpy(outVerts, &verts[0], nbVert*sizeof(alfar::Vector4));
		memcpy(outIndices, &faces[0], nbFace*sizeof(uint32));
	}
}

void alfodr::meshFromMemap(const char* file, uint32& nbVert, uint32& nbFace, SimpleVertex* &outVerts, uint32* &outIndices)
{
	std::ifstream myfile(file, std::ios::binary);

	if (myfile.is_open())
	{
		myfile.seekg(0, std::ios::end);
		std::streamsize size = myfile.tellg();
		myfile.seekg(0, std::ios::beg);

		char* buffer = (char*)malloc(size);

		myfile.read(buffer, size);

		char* original = buffer;//save for delete

		uint32 data = *(uint32*)buffer;
		buffer += sizeof(uint32);

		nbVert = *(uint32*)buffer;
		buffer += sizeof(uint32);

		outVerts = (SimpleVertex*)malloc(nbVert * sizeof(SimpleVertex));

		for(int i = 0; i < nbVert; ++i)
		{
			outVerts[i].pos = alfar::vector4::create(*((float*)buffer), *((float*)buffer+1), *((float*)buffer+2), 1.0f);
			buffer += 3 * sizeof(float);

			if(data&0x04)
			{//UV
				outVerts[i].uv = alfar::vector4::create(*((float*)buffer), *((float*)buffer+1), *((float*)buffer+2), 0.0f);
				buffer += 3 * sizeof(float);
			}

			if(data&0x2)
			{//normal
				outVerts[i].normal = alfar::vector4::create(*((float*)buffer), *((float*)buffer+1), *((float*)buffer+2), 0.0f);
				buffer += 3 * sizeof(float);
			}
		}

		nbFace = *(uint32*)buffer;
		nbFace *= 3;

		buffer += sizeof(uint32);
		outIndices = (uint32*)malloc(nbFace * sizeof(uint32));

		for(int i = 0; i < nbFace; ++i)
		{
			outIndices[i] = *((uint32*)buffer + 0);

			buffer += sizeof(uint32);
		}

		free(original);
	}
}