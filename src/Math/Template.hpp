#pragma once
#include <serializer.h>
#include <vector>
#include "SixDOF.hpp"
#include "RasterPoint.hpp"

namespace tr {
	struct Template {
		std::vector<RasterPoint> rasterPoints;

		SixDOF sixDOF;

		friend std::ostream& operator<<(std::ostream& ost, const Template& tmp);

		friend std::ostream& operator<<(std::ostream& out, Bits<Template&> o);

		friend std::istream& operator>>(std::istream& ins, Bits<Template&> o);
	};
}

