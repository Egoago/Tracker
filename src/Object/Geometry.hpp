#pragma once
#include <vector>
#include "../Misc/Base.hpp"

namespace tr {
	class Geometry {
	protected:
		std::vector<uint> indices;
		std::vector<vec3f> vertices;
		std::vector<vec3f> normals;
		std::vector<vec3f> edgeVertices;
		std::vector<float> edgeCurvatures;

	public:
		inline const uint* getIndices() const { return &indices[0]; }
		inline const vec3f* getVertices() const { return &vertices[0]; }
		inline const vec3f* getNormals() const { return &normals[0]; }
		inline const vec3f* getEdges() const { return &edgeVertices[0]; }
		inline const float* getCurvatures() const { return &edgeCurvatures[0]; }

		inline constexpr uint getIndexSize() const { return sizeof(uint); }
		inline constexpr uint getVertexSize() const { return sizeof(vec3f); }
		inline constexpr uint getNormalSize() const { return sizeof(vec3f); }

		inline uint getIndexCount() const { return (uint)indices.size(); }
		inline uint getFaceCount() const { return getIndexCount() / 3; }
		inline uint getVertexCount() const { return (uint)vertices.size(); }
		inline uint getNormalCount() const { return (uint)normals.size(); }
		inline uint getEdgeCount() const { return (uint)edgeVertices.size() / 2; }

	protected:
		void detectEdgePairs();
	};

}
