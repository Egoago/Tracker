#pragma once
#include "Geometry.h"
#include <string>

class AssimpGeometry : public Geometry
{
public:
	AssimpGeometry(const std::string& fileName);
};