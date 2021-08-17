#pragma once
#include "DistanceTensor.hpp"
#include "../Math/SixDOF.hpp"
#include "../Math/Template.hpp"
#include "../Misc/Base.hpp"
#include <ceres/jet.h>

namespace tr{
	class Registrator {
	protected:
		const mat4d P;
	public:
		Registrator(const mat4d& P) : P(P) {}
		virtual SixDOF registrate(const DistanceTensor& distanceTensor, const Template* candidate) = 0;
	};
}