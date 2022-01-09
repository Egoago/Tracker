#pragma once
#include "../Misc/Base.hpp"

namespace tr {

	template <class PointType = vec3f>
	struct Edge {
		PointType a, b;

		static const PointType reference;

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
			return a == other.a && b == other.b
				|| a == other.b && b == other.a;
		}

		inline PointType direction() const {
			PointType dir = -(a - b).matrix().normalized();
			//opposite directions are the same
			return (reference.matrix().normalized().dot(dir.matrix()) < 0.0f) ?
				dir : -dir;
		}
	};

	template<typename PointType>
	const PointType Edge<PointType>::reference = PointType::Random();
}

