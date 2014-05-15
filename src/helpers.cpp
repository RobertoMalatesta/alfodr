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