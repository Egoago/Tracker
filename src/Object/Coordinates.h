#pragma once
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <string>
#include <iostream>

struct Vertex {
	glm::vec3 position, normal;

	bool operator==(const Vertex& other) {
		const float epsilon = 1e-13f;
		return glm::distance(position, other.position) < epsilon;
	}
};

struct SixDOF {
	glm::vec3 position, orientation;

	SixDOF(glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f),
		   glm::vec3 orientation = glm::vec3(0.0f,0.0f,0.0f)) {}

	glm::mat4 getModelTransformMatrix() {
		glm::mat4 T, R;
		T = glm::translate(T, glm::vec3( position.x, position.y, position.z ));
		R = glm::rotate(R, orientation.r, glm::vec3( 0.0f, 0.0f, 1.0f));
		R = glm::rotate(R, orientation.y, glm::vec3( 0.0f, 1.0f, 0.0f));
		R = glm::rotate(R, orientation.p, glm::vec3( 1.0f, 0.0f, 0.0f));
	}

	void print(std::ostream& os) const {
		os << "pos: "
			<< position.x << ' '
			<< position.x << ' '
			<< position.y << ' '
			<< std::endl << "ori: "
			<< orientation.y << ' '
			<< orientation.p << ' '
			<< orientation.r << std::endl;
	}
};


struct Edge {
	glm::vec3 a, b;

	Edge(glm::vec3 a, glm::vec3 b) : a(a), b(b) {}
};

struct Range {
	float begin, end, step;
	unsigned int resolution;
	Range(std::vector<std::string> values) :
		begin(std::stof(values[0])),
		end(std::stof(values[1])),
		resolution(std::stoi(values[2])) {
		step = (end - begin) / resolution;
	};
};

struct Snapshot {
	SixDOF sixDOF;
	std::vector<glm::vec3> M, M_;
};
