#pragma once
#include "Loss.hpp"

namespace tr {
	class Quadratic : public Loss {
	public:
		static float loss(const float loss);
		static float der(const float loss);
	};
}