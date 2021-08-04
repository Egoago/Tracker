#pragma once
#include "../Coordinates.hpp"
#include "../DistanceTensor.hpp"

namespace tr{
	class Registrator {
		virtual SixDOF registrate(const DistanceTensor& distanceTensor, Template* candidate) = 0;
	};
}


