#pragma once
#include "Geometry.h"

class AssimpGeometry : public Geometry
{
public:
	AssimpGeometry(const char* fileName);
};

