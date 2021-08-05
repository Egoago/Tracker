#pragma once
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <serializer.h>
#include "Misc/Log.hpp"

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

	inline float getOrientation(const glm::vec2 d) {
		float angle = glm::atan(d.y / d.x);

		
		if (angle < 0.0f)
			angle += glm::pi<float>();
		return angle;
	}

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

		inline float orientation() const {
			return getOrientation(a - b);
		}

		inline bool operator==(const Edge& other) {
			const float epsilon = 1e-13f;
			return
				glm::distance(a, other.a) < epsilon &&
				glm::distance(b, other.b) < epsilon;
		}
	};

	struct Range {
		float begin, end, step, dist;
		unsigned int resolution;

		Range(std::vector<std::string> values) :
			begin(std::stof(values[0])),
			end(std::stof(values[1])),
			resolution(std::stoi(values[2])) {
			step = (end - begin) / resolution;
			dist = abs(end - begin);
		};

		float operator[](unsigned int index) const {
			if (resolution == 0u) return 0.0f;
			if (resolution == 1u) return (begin+end)/2.0f;
			float t = (float)index / (resolution - 1.0f);
			return (1.0f - t) * begin + t * end;
		}
	};

	struct Template {
		SixDOF sixDOF;
		std::vector<glm::vec3> pos, offsetPos;
		std::vector<glm::vec2> uv;
		std::vector<float> angle;

		//TODO custom serialization for smaller file size
		friend std::ostream& operator<<(std::ostream& out, Bits<Template&> o) {
			out << bits(o.t.sixDOF )
				<< bits(o.t.pos )
				<< bits(o.t.offsetPos)
				<< bits(o.t.uv)
				<< bits(o.t.angle);
			return out;
		}

		friend std::istream& operator>>(std::istream& ins, Bits<Template&> o) {
			ins >> bits(o.t.sixDOF )
				>> bits(o.t.pos )
				>> bits(o.t.offsetPos)
				>> bits(o.t.uv)
				>> bits(o.t.angle);
			return ins;
		}

		friend std::ostream& operator<<(std::ostream& ost, const Template& tmp) {
			ost << "points: " << tmp.pos.size() << std::endl
				<< "6DOF: " << tmp.sixDOF;
			return ost;
		}
	};

	inline uint64_t constexpr mix(char m, uint64_t s) {
		return ((s << 7) + ~(s >> 3)) + ~m;
	}

	inline uint64_t constexpr strHash(const char* m) {
		return (*m) ? mix(*m, strHash(m + 1)) : 0;
	}
}


