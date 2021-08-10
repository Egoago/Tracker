#pragma once
#include <Eigen/Dense>
#include <glm/matrix.hpp>
#include <ceres/rotation.h>
#include <Eigen/Geometry>

namespace tr {
	typedef unsigned int uint;

	//TODO float <-> double
	typedef float real;

	typedef Eigen::Matrix<real, 4, 4> emat4;
	typedef Eigen::Matrix<real, 2, 1> evec2;
	typedef Eigen::Matrix<real, 4, 1> evec4;
	typedef glm::mat<4, 4, real> gmat4;

	//TODO remove glm
	inline gmat4 EigenToGlmMat(const tr::emat4& v) {
		gmat4 result;
		for (uint i = 0u; i < 4u; ++i) {
			for (uint j = 0u; j < 4u; ++j) {
				result[i][j] = v(j, i);
			}
		}
		return result;
	}
	inline tr::emat4 GlmMatToEigen(const gmat4& v) {
		tr::emat4 result;
		for (uint i = 0u; i < 4u; ++i) {
			for (uint j = 0u; j < 4u; ++j) {
				result(j, i) = v[i][j];
			}
		}
		return result;
	}

	//TODO use quat vs euler
	template<typename T>
	Eigen::Quaternion<T> RPYToQ(const T rotation[3]) {
		return Eigen::AngleAxisf(rotation[2], Eigen::Vector3f::UnitX())
			 * Eigen::AngleAxisf(rotation[1], Eigen::Vector3f::UnitY())
			 * Eigen::AngleAxisf(rotation[0], Eigen::Vector3f::UnitZ());
	}

	inline uint64_t constexpr mix(char m, uint64_t s) {
		return ((s << 7) + ~(s >> 3)) + ~m;
	}

	inline uint64_t constexpr strHash(const char* m) {
		return (*m) ? mix(*m, strHash(m + 1)) : 0;
	}
}


