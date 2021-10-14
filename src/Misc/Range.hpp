#pragma once
#include <string>
#include <vector>
#include "Base.hpp"

namespace tr {
	class Range {


		float begin, end, dist, k = 1.0f;
		uint resolution;
		bool cyclic = false;
		float (*eval)(float, float);

		static float linear(float x, float k = 0.0f) { return x; }
		static float exponential(float x, float k = 1.0f) { return powf(10.0f, k *(x- 1))- powf(10.0f, - k); }
		static float polinomial(float x, float k = 2.0f) { return powf(x, k); }

		float t(uint index) const {
			if (resolution == 0u) return 0.0f;
			if (resolution == 1u) return 0.5f;
			if (cyclic) return (index % resolution) / (float)resolution;
			else return (float)(index % resolution) / (resolution - 1.0f);
		}
	public:

		enum class Interploation {
			LINEAR, EXPONENTIAL, POLINOMIAL
		};

		Range(std::vector<int> values, Interploation interp = Interploation::LINEAR, bool cyclic = false) :
			Range(float(values[0]), float(values[1]), values[2], interp, cyclic) {}

		Range(float min, float max, uint resolution, Interploation interp = Interploation::LINEAR, bool cyclic = false) :
			begin(min), end(max), resolution(resolution), cyclic(cyclic) {
			dist = max - min;
			switch (interp) {
			case Interploation::LINEAR: eval = linear; k = 0.0f; break;
			case Interploation::EXPONENTIAL: eval = exponential; k = 1.0f;  break;
			case Interploation::POLINOMIAL: eval = polinomial; k = 2.0f; break;
			default: eval = linear; k = 0.0f; break;
			}
		};

		inline void setK(float k) { this->k = k; }
		
		inline void setCyclic(bool cyclic = true) { this->cyclic = cyclic; }

		inline uint size() const { return resolution; }

		inline float operator[](uint index) const {
			return dist * eval(t(index), k) + begin;
		}

	};
}