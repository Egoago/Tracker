#pragma once
#include "Geometry.h"
#include <string>

namespace tr
{
	class AssimpGeometry : public Geometry
	{
	public:
		AssimpGeometry(const std::string& fileName);
	};
}