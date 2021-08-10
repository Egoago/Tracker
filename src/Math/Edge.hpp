#pragma once
#include <glm/vec3.hpp>
#include "Rotation.hpp"

namespace tr {
	template <class PointType = glm::vec3>
	struct Edge {
		PointType a, b;

		Edge(PointType a = PointType(), PointType b = PointType()) : a(a), b(b) {}

		Edge& flip() {
			const PointType c = a;
			a = b;
			b = c;
			return *this;
		}

		inline float orientation() const {
			return getOrientation(a - b);
		}

		inline bool operator==(const Edge& other) {
			const float epsilon = 1e-13f;
			return
				glm::distance(a, other.a) < epsilon &&
				glm::distance(b, other.b) < epsilon;
		}
	};
}

