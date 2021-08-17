#pragma once
#include "../Misc/Base.hpp"

namespace tr {
	template <class PointType = vec3f>
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
			return tr::orientation(a,b);
		}

		inline bool operator==(const Edge& other) {
			return a == other.a && b == other.b;
		}
	};
}

