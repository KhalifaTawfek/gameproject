#include "meshLoaderObj.h"
#include "stringTokenizer.h"
#include <iostream>
#include <fstream>

MeshLoaderObj::MeshLoaderObj() {};

Mesh MeshLoaderObj::loadObj(const std::string& filename)
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
	if (!file.good())
	{
		std::cout << "Obj model not found " << filename << std::endl;
		std::terminate();
	}

	std::string line;
	std::vector<std::string> tokens, facetokens;

	std::vector<glm::vec3> positions;
	positions.reserve(1000);

	std::vector<glm::vec3> normals;
	normals.reserve(1000);

	std::vector<glm::vec2> texcoords;
	texcoords.reserve(1000);

	while (std::getline(file, line))
	{
		_stringTokenize(line, tokens);

		if (tokens.size() == 0)
			continue;

		if (tokens.size() > 0 && tokens[0].at(0) == '#')
			continue;

		if (tokens.size() > 3 && tokens[0] == "v")
			positions.push_back(glm::vec3(_stringToFloat(tokens[1]), _stringToFloat(tokens[2]), _stringToFloat(tokens[3])));

		if (tokens.size() > 3 && tokens[0] == "vn")
			normals.push_back(glm::vec3(_stringToFloat(tokens[1]), _stringToFloat(tokens[2]), _stringToFloat(tokens[3])));

		if (tokens.size() > 2 && tokens[0] == "vt")
			texcoords.push_back(glm::vec2(_stringToFloat(tokens[1]), _stringToFloat(tokens[2])));

		if (tokens.size() >= 4 && tokens[0] == "f")
		{
			unsigned int face_format = 0;
			if (tokens[1].find("//") != std::string::npos) face_format = 3;
			_faceTokenize(tokens[1], facetokens);

			if (facetokens.size() == 3)
				face_format = 4;
			else
			{
				if (facetokens.size() == 2)
				{
					if (face_format != 3) face_format = 2;
				}
				else
				{
					face_format = 1;
				}
			}

			unsigned int index_of_first_vertex_of_face = -1;

			for (unsigned int num_token = 1; num_token < tokens.size(); num_token++)
			{
				if (tokens[num_token].at(0) == '#') break;
				_faceTokenize(tokens[num_token], facetokens);

				glm::vec3 tempPos(0.0f);
				glm::vec2 tempTex(0.0f);
				glm::vec3 tempNorm(0.0f);

				int p_index = _stringToInt(facetokens[0]);
				if (p_index > 0) p_index -= 1; else p_index = positions.size() + p_index;

				if (p_index >= 0 && p_index < positions.size())
					tempPos = positions[p_index];

				if (face_format == 1) 
				{
					vertices.push_back(Vertex(tempPos.x, tempPos.y, tempPos.z));
				}
				else if (face_format == 2) 
				{
					int t_index = _stringToInt(facetokens[1]);
					if (t_index > 0) t_index -= 1; else t_index = texcoords.size() + t_index;

					if (t_index >= 0 && t_index < texcoords.size())
						tempTex = texcoords[t_index];

					vertices.push_back(Vertex(tempPos.x, tempPos.y, tempPos.z, tempTex.x, tempTex.y));
				}
				else if (face_format == 3) 
				{
					int n_index = _stringToInt(facetokens[1]);
					if (n_index > 0) n_index -= 1; else n_index = normals.size() + n_index;

					if (n_index >= 0 && n_index < normals.size())
						tempNorm = normals[n_index];

					vertices.push_back(Vertex(tempPos.x, tempPos.y, tempPos.z, tempNorm.x, tempNorm.y, tempNorm.z));
				}
				else 
				{
					int t_index = _stringToInt(facetokens[1]);
					if (t_index > 0) t_index -= 1; else t_index = texcoords.size() + t_index;

					int n_index = _stringToInt(facetokens[2]);
					if (n_index > 0) n_index -= 1; else n_index = normals.size() + n_index;

					if (t_index >= 0 && t_index < texcoords.size())
						tempTex = texcoords[t_index];
					if (n_index >= 0 && n_index < normals.size())
						tempNorm = normals[n_index];

					vertices.push_back(Vertex(tempPos.x, tempPos.y, tempPos.z, tempNorm.x, tempNorm.y, tempNorm.z, tempTex.x, tempTex.y));
				}

				if (num_token < 4)
				{
					if (num_token == 1)
						index_of_first_vertex_of_face = vertices.size() - 1;

					indices.push_back(vertices.size() - 1);
				}
				else
				{
					indices.push_back(index_of_first_vertex_of_face);
					indices.push_back(vertices.size() - 2);
					indices.push_back(vertices.size() - 1);
				}
			}
		}
	}

	std::cout << "Loading:  " << filename << std::endl;

	Mesh mesh(vertices, indices);

	return mesh;
}

Mesh MeshLoaderObj::loadObj(const std::string& filename, std::vector<Texture> textures)
{
	Mesh mesh = loadObj(filename);
	mesh.setTextures(textures);

	return mesh;
}