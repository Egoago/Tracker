#pragma once
#include <Eigen/Dense>
#include <glm/matrix.hpp>
#include <ceres/rotation.h>
#include <opencv2/core/hal/interface.h>

namespace tr {
	typedef unsigned int uint;

	//TODO float <-> double
	typedef double real;
	const uint realCV = (std::is_same<real, double>::value) ? CV_64F : CV_32F;

	typedef Eigen::Matrix<real, 4, 4> emat4;
	typedef Eigen::Matrix<real, 2, 1> evec2;
	typedef Eigen::Matrix<real, 3, 1> evec3;
	typedef Eigen::Matrix<real, 4, 1> evec4;
	typedef glm::mat<4, 4, real> gmat4;
	typedef glm::vec<3, real> gvec3;

	//TODO remove glm
	template<typename gT, typename eT, uint m, uint n>
	inline glm::mat<m, n, gT> E2GLM(const Eigen::Matrix<eT, m, n>& em) {
		glm::mat<m, n, gT> gm;
		for (uint i = 0; i < m; ++i)
			for (uint j = 0; j < n; ++j)
				gm[j][i] = gT(em(i, j));
		return gm;
	}
	template<typename gT, typename eT, uint m>
	inline glm::vec<m, gT> E2GLM(const Eigen::Matrix<eT, m, 1>& em) {
		glm::vec<m, gT> gm;
		for (uint i = 0; i < m; ++i)
				gm[i] = gT(em[i]);
		return gm;
	}
	template<typename eT, typename gT, uint m, uint n>
	inline Eigen::Matrix<eT, m, n> GLM2E(const glm::mat<m, n, gT>& gm) {
		Eigen::Matrix<eT, m, n> em;
		for (uint i = 0; i < m; ++i)
			for (uint j = 0; j < n; ++j)
				em(i, j) = eT(gm[j][i]);
		return em;
	}
	template<typename eT, typename gT, uint m>
	inline Eigen::Matrix<eT, m, 1> GLM2E(const glm::vec<m, gT>& gm) {
		Eigen::Matrix<eT, m, 1> em;
		for (uint i = 0; i < m; ++i)
				em[i] = eT(gm[i]);
		return em;
	}

	//TODO use quat vs euler
	template<typename T>
	Eigen::Quaternion<T> RPYToQ(const T rotation[3]) {
		return Eigen::AngleAxis<T>(rotation[2], Eigen::Matrix<T, 3, 1>::UnitX())
			 * Eigen::AngleAxis<T>(rotation[1], Eigen::Matrix<T, 3, 1>::UnitY())
			 * Eigen::AngleAxis<T>(rotation[0], Eigen::Matrix<T, 3, 1>::UnitZ());
	}

	inline uint64_t constexpr mix(char m, uint64_t s) {
		return ((s << 7) + ~(s >> 3)) + ~m;
	}

	inline uint64_t constexpr strHash(const char* m) {
		return (*m) ? mix(*m, strHash(m + 1)) : 0;
	}
}


