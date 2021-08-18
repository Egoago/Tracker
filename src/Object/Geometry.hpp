#pragma once
#include <vector>
#include "../Misc/Base.hpp"

namespace tr {
	struct Geometry {
		std::vector<uint> indices;
		std::vector<vec3f> vertices;
		std::vector<vec3f> normals;
		std::vector<vec3f> lowEdgeVertices;
		std::vector<vec3f> highEdgeVertices;
		std::vector<float> lowEdgeCurvatures;
		std::vector<float> highEdgeCurvatures;

		void generate();
	};

}
