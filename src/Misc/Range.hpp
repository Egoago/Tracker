#pragma once
#include <string>
#include <vector>
#include "Base.hpp"

namespace tr {
	struct Range {
		float begin, end, step, dist;
		uint resolution;

		Range(std::vector<int> values) :
			begin(float(values[0])),
			end(float(values[1])),
			resolution(values[2]) {
			step = (end - begin) / resolution;
			dist = abs(end - begin);
		};

		float operator[](uint index) const {
			if (resolution == 0u) return 0.0f;
			if (resolution == 1u) return (begin + end) / 2.0f;
			float t = (float)index / (resolution - 1.0f);
			return (1.0f - t) * begin + t * end;
		}
	};
}