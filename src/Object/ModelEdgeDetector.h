#pragma once
#include "Geometry.h"

class ModelEdgeDetector
{
public:
	std::vector<Edge> detectOutlinerEdges(Geometry& geometry);
};

