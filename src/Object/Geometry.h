#pragma once
#include <vector>
#include "Coordinates.h"

class Geometry
{
protected:
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> vertices;

public:
	const glm::vec3* getVertices() const { return vertices.data(); }
	const unsigned int* getIndices() const { return indices.data(); }
	constexpr unsigned int getIndexSize() const { return sizeof(unsigned int); }
	unsigned int getIndecesCount() const { return indices.size(); }
	constexpr unsigned int getVertexSize() const { return sizeof(glm::vec3); }
	unsigned int getVerticesCount() const { return vertices.size(); }
};

