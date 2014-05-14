#include "helpers.h"
#include "renderer.h"

#include <vector4.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace alfodr;

void alfodr::meshFromFile(const char* file, uint32& nbVert, uint32& nbFace, alfar::Vector4* &outVerts, uint32* &outIndices, uint32& strides)
{
	std::ifstream myfile (file);

	if (myfile.is_open())
	{
		std::string line;
		std::vector<alfar::Vector4> verts, uvs;
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
			else if(c == "f")
			{
				uint32 v1,v2,v3;
				myfile>>v1>>c>>v2>>v3;

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

		strides = sizeof(alfar::Vector4);
		strides += sizeof(alfar::Vector4);

		outVerts = (alfar::Vector4*)malloc(nbVert * strides);
		outIndices = (uint32*)malloc(nbFace * sizeof(uint32));

		for(int i = 0; i < nbVert; ++i)
		{
			outVerts[i*2] = verts[i];
			outVerts[i*2+1] = uvs[i];
		}

		//memcpy(outVerts, &verts[0], nbVert*sizeof(alfar::Vector4));
		memcpy(outIndices, &faces[0], nbFace*sizeof(uint32));
	}
}