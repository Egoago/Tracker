#pragma once
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <serializer.h>

struct SixDOF {
	glm::vec3 position, orientation;

	SixDOF(glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f),
		   glm::vec3 orientation = glm::vec3(0.0f,0.0f,0.0f)) :
		position(position), orientation(orientation) {}

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
			<< position.y << ' '
			<< position.z << ' '
			<< std::endl << "ori: "
			<< orientation.y << ' '
			<< orientation.p << ' '
			<< orientation.r << std::endl;
	}
};

template <class PointType = glm::vec3>
struct Edge {
	PointType a, b;

	Edge(PointType a = PointType(), PointType b = PointType()) : a(a), b(b) {}

	Edge& flip() {
		const PointType c = a;
		a = b;
		b = c;
		return *this;
	}

	bool operator==(const Edge& other) {
		const float epsilon = 1e-13f;
		return
			glm::distance(a, other.a) < epsilon &&
			glm::distance(b, other.b) < epsilon;
	}
};

inline float getOrientation(const Edge<glm::vec2>& edge) {
	glm::vec2 d = edge.a - edge.b;
	float angle = glm::atan(d.y / d.x);
	if (angle < 0.0f)
		angle += glm::pi<float>();
	return angle;
}

struct Range {
	float begin, end, step;
	unsigned int resolution;
	Range(std::vector<std::string> values) :
		begin(std::stof(values[0])),
		end(std::stof(values[1])),
		resolution(std::stoi(values[2])) {
		step = (end - begin) / resolution;
	};

	float operator[](unsigned int index) {
		float t = (float)index / (resolution-1.0f);
		return (1.0f-t)*begin + t*end;
	}
};

struct Template {
	SixDOF sixDOF;
	std::vector<glm::vec3> M, M_;

	friend std::ostream& operator<<(std::ostream& out, Bits<struct Template&> o) {
		out << bits(o.t.sixDOF)
			<< bits(o.t.M)
			<< bits(o.t.M_);
		return (out);
	}

	friend std::istream& operator>>(std::istream& in, Bits<struct Template&> o) {
		in >> bits(o.t.sixDOF)
			>> bits(o.t.M)
			>> bits(o.t.M_);
		return (in);
	}
};

constexpr inline int quantizedIndex(const float value, const unsigned int q) {
	const constexpr float pi = glm::pi<float>();
	const float d = pi / q;
	return (int)(value / d) % q;
}
