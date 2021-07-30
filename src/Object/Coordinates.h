#pragma once
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <serializer.h>
#include "../Misc/Log.h"

//TODO move everything to tr
namespace tr {
	struct SixDOF {
		glm::vec3 position, orientation;

		SixDOF(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f)) :
			position(position), orientation(orientation) {}

		glm::mat4 getModelTransformMatrix() {
			glm::mat4 T, R;
			T = glm::translate(T, glm::vec3(position.x, position.y, position.z));
			R = glm::rotate(R, orientation.r, glm::vec3(0.0f, 0.0f, 1.0f));
			R = glm::rotate(R, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			R = glm::rotate(R, orientation.p, glm::vec3(1.0f, 0.0f, 0.0f));
		}

		friend std::ostream& operator<<(std::ostream& ost, const SixDOF& sDOF) {
			ost << "pos: "
				<< sDOF.position.x << ' '
				<< sDOF.position.y << ' '
				<< sDOF.position.z << ' '
				<< "ori: "
				<< sDOF.orientation.y << ' '
				<< sDOF.orientation.p << ' '
				<< sDOF.orientation.r << std::endl;
			return ost;
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
			float t = (float)index / (resolution - 1.0f);
			return (1.0f - t) * begin + t * end;
		}
	};

	struct Template {
		SixDOF sixDOF;
		std::vector<glm::vec3> pos, offsetPos;

		friend std::ostream& operator<<(std::ostream& out, Bits<Template&> o) {
			out << bits(o.t.sixDOF )<< bits(o.t.pos )<< bits(o.t.offsetPos);
			return out;
		}

		friend std::istream& operator>>(std::istream& ins, Bits<Template&> o) {
			ins >> bits(o.t.sixDOF )>> bits(o.t.pos )>> bits(o.t.offsetPos);
			return ins;
		}

		friend std::ostream& operator<<(std::ostream& ost, const Template& tmp) {
			ost << "points: " << tmp.pos.size() << std::endl
				<< "6DOF: " << tmp.sixDOF;
			return ost;
		}
	};

	constexpr inline int quantizedIndex(const float value, const unsigned int q) {
		const constexpr float pi = glm::pi<float>();
		const float d = pi / q;
		return (int)(value / d) % q;
	}
}
