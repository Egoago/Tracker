#pragma once
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <serializer.h>

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

	/*friend std::ostream& operator<<(std::ostream& out, Bits<struct SixDOF&> o)
	{
		out << bits(o.t.position)
			<< bits(o.t.orientation);
		return (out);
	}
	friend std::istream& operator>>(std::istream& in, Bits<SixDOF&> o)
	{
		in  >> bits(o.t.position)
			>> bits(o.t.orientation);
		return (in);
	}*/
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

	float operator[](size_t index) {
		float t = (float)index / (resolution-1.0f);
		return (1.0f-t)*begin + t*end;
	}
};

struct Snapshot {
	SixDOF sixDOF;
	std::vector<glm::vec3> M, M_;

	//friend std::ostream& operator<<(std::ostream& out, Bits<struct Snapshot&> o) {
	//	out << bits(o.t.sixDOF)
	//		<< bits(o.t.M.size());
	//	std::cout << "M size: " << o.t.M.size() << std::endl;
	//	/*for (size_t i = 0; i < o.t.M.size(); i++) {
	//		out << bits(o.t.M[i]);
	//		out << bits(o.t.M_[i]);
	//	}*/
	//	return (out);
	//}

	//friend std::istream& operator>>(std::istream& in, Bits<struct Snapshot&> o) {
	//	size_t size;
	//	in  >> bits(o.t.sixDOF)
	//		>> bits(size);
	//	std::cout << "M size: " << size << std::endl;
	//	o.t.M.resize(size);
	//	o.t.M_.resize(size);
	//	/*for (size_t i = 0; i < size; i++) {
	//		in >> bits(o.t.M[i]);
	//		in >> bits(o.t.M_[i]);
	//	}*/
	//	return (in);
	//}
};


