#include "Rasterizer.h"

Rasterizer::Rasterizer(const std::vector<Edge>& edges, const float step, const float d)
{
	for(const Edge& edge : edges)
	{
		glm::vec3 dir = glm::normalize(edge.b - edge.a);
		float dist = glm::distance(edge.a, edge.b);
		glm::vec3 p = edge.a;
		M.push_back(p);
		while (dist > 0) {
			M.push_back(p);
			M_.push_back(p + d * dir);
			p += step * dir;
			dist -= step;
		}
	}
}
