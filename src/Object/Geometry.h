#pragma once
#include <vector>

struct Vec3 {
	float x = 0, y = 0, z = 0;

	bool operator==(const Vec3& other) {
		const float epsilon = 1e-13f;
		return
			abs(x - other.x) < epsilon &&
			abs(y - other.y) < epsilon &&
			abs(z - other.z) < epsilon;
	}
};

struct Vertex {
	Vec3 position, normal;

	bool operator==(const Vertex& other) {
		return position == other.position;
	}
};

struct Edge {
	Vec3 a, b;
};

class Geometry
{
protected:
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

