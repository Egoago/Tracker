#pragma once
#include <vector>

class Geometry
{
protected:
	struct Vec3 {
		float x, y, z;
	};

	struct Vertex {
		Vec3 position, normal;
	};
	
	std::vector<unsigned int> indices;
	std::vector<Vertex> vertices;

public:
	const void* getVertices() const { return &vertices[0]; }
	const void* getIndices() const { return &indices[0]; }
	size_t getIndexSize() const { return sizeof(unsigned int); }
	size_t getIndecesCount() const { return indices.size(); }
	size_t getVertexSize() const { return sizeof(Vertex); }
	size_t getVerticesCount() const { return vertices.size(); }
};

