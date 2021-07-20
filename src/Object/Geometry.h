#pragma once
#include <vector>
#include <glm/ext/vector_float3.hpp>

class Geometry
{
protected:
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> edgeVertices;

public:
	inline const unsigned int* getIndices() const { return &indices[0]; }
	inline const glm::vec3* getVertices() const { return &vertices[0]; }
	inline const glm::vec3* getNormals() const { return &normals[0]; }
	inline const glm::vec3* getEdges() const { return &edgeVertices[0]; }

	inline constexpr unsigned int getIndexSize() const { return sizeof(unsigned int); }
	inline constexpr unsigned int getVertexSize() const { return sizeof(glm::vec3); }
	inline constexpr unsigned int getNormalSize() const { return sizeof(glm::vec3); }
	
	inline unsigned int getIndexCount() const { return (unsigned int)indices.size(); }
	inline unsigned int getFaceCount() const { return getIndexCount() /3; }
	inline unsigned int getVertexCount() const { return (unsigned int)vertices.size(); }
	inline unsigned int getNormalCount() const { return (unsigned int)normals.size(); }
	inline unsigned int getEdgeCount() const { return (unsigned int)edgeVertices.size()/2; }

protected:
	void detectEdgePairs();
};

