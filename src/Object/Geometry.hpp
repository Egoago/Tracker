#pragma once
#include <vector>
#include "../Misc/Base.hpp"

namespace tr {
	struct Geometry {
		//Raw
		std::vector<uint> indices;
		std::vector<vec3f> vertices;
		std::vector<vec3f> normals;
		//Computed
		std::vector<vec3f> edges;	//vec3f a1,dir1,b1,dir1,a2,dir2,b2,dir2... interleaved
		std::vector<uint> highEdgeIndices;
		std::vector<uint> lowEdgeIndices;
		float boundingRadius = 0.0f;
		vec3f centerOffset;

		void generate();
	};
}
