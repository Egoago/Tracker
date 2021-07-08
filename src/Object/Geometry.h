#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

struct Vertex {
	glm::vec3 position, normal;

	bool operator==(const Vertex& other) {
		const float epsilon = 1e-13f;
		return glm::distance(position, other.position) < epsilon;
	}
};

struct Edge {
	glm::vec3 a, b;

	Edge(glm::vec3 a, glm::vec3 b) : a(a), b(b) {}
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

