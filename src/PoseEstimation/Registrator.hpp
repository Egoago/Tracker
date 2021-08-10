#pragma once
#include "DistanceTensor.hpp"
#include "../Math/SixDOF.hpp"
#include "../Math/Template.hpp"
#include "../Misc/Base.hpp"
#include <ceres/jet.h>

namespace tr{
	class Registrator {
		emat4 P;
		
	public:
		template<typename T>
		inline void project(const T transf[6], const real point[3], T uv[2]) {
			Eigen::Quaternion<T> q = RPYToQ(&transf[3]);
			Eigen::Map<const Eigen::Matrix<real, 3, 1>> p(point);
			Eigen::Map<const Eigen::Matrix<T, 3, 1>> t(transf);
			const auto cam = q * p + t;
			Eigen::Matrix<T, 3, 1> proj = (P.cast<T>() * cam.homogeneous()).hnormalized();
			for (uint i = 0; i < 2; i++)
				uv[i] = (proj[i] + T(1)) / T(2);
		}
		Registrator(const emat4& P) : P(P) {}
		virtual SixDOF registrate(const DistanceTensor& distanceTensor, const Template* candidate) = 0;
	};
}


