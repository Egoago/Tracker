#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace tr {
	typedef unsigned int uint;
	typedef unsigned char uchar;
	typedef unsigned long ulong;

	typedef Eigen::Matrix<float, 4, 4> mat4f;
	typedef Eigen::Matrix<double, 4, 4> mat4d;
	typedef Eigen::Array<float, 2, 1> vec2f;
	typedef Eigen::Array<double, 2, 1> vec2d;
	typedef Eigen::Array<uint, 2, 1> uvec2;
	typedef Eigen::Array<float, 3, 1> vec3f;
	typedef Eigen::Array<double, 3, 1> vec3d;
	typedef Eigen::Array<float, 4, 1> vec4f;
	typedef Eigen::Array<double, 4, 1> vec4d;

	inline uint64_t constexpr mix(char m, uint64_t s) {
		return ((s << 7) + ~(s >> 3)) + ~m;
	}

	inline uint64_t constexpr strHash(const char* m) {
		return (*m) ? mix(*m, strHash(m + 1)) : 0;
	}

	template <typename Type, int Dim = 2>
	inline Type orientation(const Eigen::Array<Type, Dim, 1>& a, const Eigen::Array<Type, Dim, 1>& b) {
		return orientation((a - b).eval());
	}
	template <typename Type, int Dim = 2>
	inline Type orientation(const Eigen::Array<Type, Dim, 1> d) {
		Type angle = atan(d.y() / d.x());
		if (angle < Type(0))
			angle += Type(EIGEN_PI);
		return angle;
	}

	template <typename Type, int Size>
	inline Type distance(const Eigen::Matrix<Type, Size, 1>& a, const Eigen::Matrix<Type, Size, 1>& b) {
		return (a-b).norm();
	}
	template <typename Type, int Size>
	inline Type distance(const Eigen::Array<Type, Size, 1>& a, const Eigen::Array<Type, Size, 1>& b) {
		return (a.matrix() - b.matrix()).norm();
	}

	template <typename Type, int Size>
	inline bool operator==(const Eigen::Array<Type, Size, 1>& a, const Eigen::Array<Type, Size, 1>& b) {
		constexpr const Type epsilon = Type(1e-7);
		for (uint i = 0; i < Size; i++)
			if (abs(a[i] - b[i]) > epsilon) return false;
		return true;
	}
	
	template <typename Type>
	inline Type angle(const Eigen::Array<Type, 3, 1> a, const Eigen::Array<Type, 3, 1> b) {
		return angle(a.matrix().eval(), b.matrix().eval());
	}
	template <typename Type>
	inline Type angle(const Eigen::Matrix<Type, 3, 1> a, const Eigen::Matrix<Type, 3, 1> b) {
		Type angle = std::atan(a.cross(b).norm()/ a.dot(b));
		return (angle < Type(0)) ? angle + Type(EIGEN_PI) : angle;
	}

	template <typename Type>
	inline Type degree(const Type radian) { return radian * Type(180) / Type(EIGEN_PI); }
	template <typename Type>
	inline Type radian(const Type degree) { return degree * Type(EIGEN_PI) / Type(180); }

}


