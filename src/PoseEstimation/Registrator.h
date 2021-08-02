#pragma once
#include "../Coordinates.h"
#include "../DistanceTensor.h"

namespace tr{
	class Registrator {
		virtual SixDOF registrate(const DistanceTensor& distanceTensor, Template* candidate) = 0;
	};
}


