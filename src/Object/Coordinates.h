#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

struct Vertex {
	glm::vec3 position, normal;

	bool operator==(const Vertex& other) {
		const float epsilon = 1e-13f;
		return glm::distance(position, other.position) < epsilon;
	}
};

struct SixDOF {
	glm::vec3 position, orientation;
};

struct Edge {
	glm::vec3 a, b;

	Edge(glm::vec3 a, glm::vec3 b) : a(a), b(b) {}
};

struct Range {
	float begin, resolution, end;
	Range(std::vector<std::string> values) :
		begin(std::stof(values[0])),
		end(std::stof(values[1])),
		resolution(std::stof(values[2])) {};
};
