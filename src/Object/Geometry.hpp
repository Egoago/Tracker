#pragma once
#include <vector>
#include <glm/ext/vector_float3.hpp>
#include "../Misc/Base.hpp"

namespace tr {
	class Geometry {
	protected:
		std::vector<uint> indices;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> edgeVertices;
		std::vector<float> edgeCurvatures;

	public:
		inline const uint* getIndices() const { return &indices[0]; }
		inline const glm::vec3* getVertices() const { return &vertices[0]; }
		inline const glm::vec3* getNormals() const { return &normals[0]; }
		inline const glm::vec3* getEdges() const { return &edgeVertices[0]; }
		inline const float* getCurvatures() const { return &edgeCurvatures[0]; }

		inline constexpr uint getIndexSize() const { return sizeof(uint); }
		inline constexpr uint getVertexSize() const { return sizeof(glm::vec3); }
		inline constexpr uint getNormalSize() const { return sizeof(glm::vec3); }

		inline uint getIndexCount() const { return (uint)indices.size(); }
		inline uint getFaceCount() const { return getIndexCount() / 3; }
		inline uint getVertexCount() const { return (uint)vertices.size(); }
		inline uint getNormalCount() const { return (uint)normals.size(); }
		inline uint getEdgeCount() const { return (uint)edgeVertices.size() / 2; }

	protected:
		void detectEdgePairs();
	};

}
