#pragma once
#include <vector>
#include "Coordinates.h"

class Geometry
{
protected:
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> vertices;

public:
	inline const glm::vec3* getVertices() const { return &vertices[0]; }
	inline const unsigned int* getIndices() const { return &indices[0]; }
	inline constexpr unsigned int getIndexSize() const { return sizeof(unsigned int); }
	inline unsigned int getIndecesCount() const { return (unsigned int)indices.size(); }
	inline unsigned int getFacesCount() const { return getIndecesCount() /3; }
	inline constexpr unsigned int getVertexSize() const { return sizeof(glm::vec3); }
	inline unsigned int getVerticesCount() const { return (unsigned int)vertices.size(); }
};

