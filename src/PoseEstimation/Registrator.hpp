#pragma once
#include "DistanceTensor.hpp"
#include "../Math/SixDOF.hpp"
#include "../Math/Template.hpp"
#include "../Misc/Base.hpp"
#include <ceres/jet.h>

namespace tr{
	class Registrator {
	protected:
		const emat4 P;
		
	public:
		Registrator(const emat4& P) : P(P) {}
		virtual SixDOF registrate(const DistanceTensor& distanceTensor, const Template* candidate) = 0;
	};
}


