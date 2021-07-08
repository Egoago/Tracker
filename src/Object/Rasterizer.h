#pragma once
#include <vector>
#include "Geometry.h"

class Rasterizer
{
private:
	std::vector<glm::vec3> M, M_;
public:
	Rasterizer(const std::vector<Edge>& edges, const float step = 2.0f, const float d = 1e-1f);
	std::vector<glm::vec3>& getM() { return M; }
	std::vector<glm::vec3>& getM_() { return M_; }
};

