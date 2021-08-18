#pragma once
#include "Geometry.hpp"
#include <string>

namespace tr {
	namespace AssimpLoader {
		void load(const std::string& fileName, Geometry& geometry);
	};
}