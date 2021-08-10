#pragma once
#include "Geometry.hpp"
#include <string>

namespace tr {
	class AssimpGeometry : public Geometry
	{
	public:
		AssimpGeometry(const std::string& fileName);
	};
}