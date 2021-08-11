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
	typedef Eigen::Matrix<real, 3, 1> evec3;
	typedef Eigen::Matrix<real, 4, 1> evec4;
	typedef glm::mat<4, 4, real> gmat4;

	//TODO remove glm
	template<typename T, uint m, uint n>
	inline glm::mat<m, n, T> E2GLM(const Eigen::Matrix<T, m, n>& em) {
		glm::mat<m, n, T> gm;
		for (uint i = 0; i < m; ++i)
			for (uint j = 0; j < n; ++j)
				gm[j][i] = em(i, j);
		return gm;
	}
	template<typename T, uint m>
	inline glm::vec<m, T> E2GLM(const Eigen::Matrix<T, m, 1>& em) {
		glm::vec<m, T> gm;
		for (uint i = 0; i < m; ++i)
				gm[i] = em[i];
		return gm;
	}
	template<typename T, uint m, uint n>
	inline Eigen::Matrix<T, m, n> GLM2E(const glm::mat<m, n, T>& gm) {
		Eigen::Matrix<T, m, n> em;
		for (uint i = 0; i < m; ++i)
			for (uint j = 0; j < n; ++j)
				em(i, j) = gm[j][i];
		return em;
	}
	template<typename T, uint m>
	inline Eigen::Matrix<T, m, 1> GLM2E(const glm::vec<m, T>& gm) {
		Eigen::Matrix<T, m, 1> em;
		for (uint i = 0; i < m; ++i)
				em[i] = gm[i];
		return em;
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


