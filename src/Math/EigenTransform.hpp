#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace tr {
	//TODO use quat vs euler
	template<typename T>
	inline Eigen::Quaternion<T> RPYToQ(const T rotation[3]) {
		return Eigen::AngleAxis<T>(rotation[0], Eigen::Matrix<T, 3, 1>::UnitY())
			* Eigen::AngleAxis<T>(rotation[1], Eigen::Matrix<T, 3, 1>::UnitX())
			* Eigen::AngleAxis<T>(rotation[2], Eigen::Matrix<T, 3, 1>::UnitZ());
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> perspective(const Scalar fovy, const Scalar aspect, const Scalar zNear, const Scalar zFar) {
		Eigen::Transform<Scalar, 3, Eigen::Projective> tr;
		tr.matrix().setZero();
		const Scalar tanHalfFov = std::tan(fovy / Scalar(2));
		tr(0, 0) = Scalar(1) / (aspect * tanHalfFov);
		tr(1, 1) = Scalar(1) / (tanHalfFov);
		tr(2, 2) = -(zFar + zNear) / (zFar - zNear);
		tr(3, 2) = Scalar(-1);
		tr(2, 3) = -(Scalar(2) * zFar * zNear) / (zFar - zNear);
		return tr.matrix();
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> scale(const Scalar x, const Scalar y, const Scalar z) {
		Eigen::Transform<Scalar, 3, Eigen::Affine> tr;
		tr.matrix().setZero();
		tr(0, 0) = x;
		tr(1, 1) = y;
		tr(2, 2) = z;
		tr(3, 3) = Scalar(1);
		return tr.matrix();
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> translate(const Eigen::Array<Scalar, 3, 1>& t) {
		Eigen::Matrix<Scalar, 4, 4> tr = Eigen::Matrix<Scalar, 4, 4>::Identity();
		tr.block<3, 1>(0, 3) = t;
		return tr;
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> translate(const Scalar x, const Scalar y, const Scalar z) {
		Eigen::Transform<Scalar, 3, Eigen::Affine> tr;
		tr.matrix().setIdentity();
		tr(0, 3) = x;
		tr(1, 3) = y;
		tr(2, 3) = z;
		return tr.matrix();
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> rotate(const Eigen::Quaternion<Scalar>& q) {
		Eigen::Transform<Scalar, 3, Eigen::Affine> tr;
		tr.matrix() = q.normalized().toRotationMatrix();
		return tr.matrix();
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> rotate(const Scalar yaw, const Scalar pitch, const Scalar roll) {
		Eigen::Transform<Scalar, 3, Eigen::Affine> t;
		t.matrix().setIdentity();
		t.rotate(Eigen::AngleAxis<Scalar>(yaw, Eigen::Matrix<Scalar, 3, 1>::UnitY()));
		t.rotate(Eigen::AngleAxis<Scalar>(pitch, Eigen::Matrix<Scalar, 3, 1>::UnitX()));
		t.rotate(Eigen::AngleAxis<Scalar>(roll, Eigen::Matrix<Scalar, 3, 1>::UnitZ()));
		return t.matrix();
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> rotate(const Eigen::Array<Scalar, 3, 1>& ypr) {
		return rotate(ypr[0], ypr[1], ypr[2]);
	}

	template<typename T, typename PType>
	inline bool renderPoint(const T pos[3], const T ori[3], const Eigen::Array<PType,3,1> p, T uv[2], const Eigen::Matrix<PType, 4, 4>& P) {
		//Logger::logProcess(__FUNCTION__);   //TODO remove logging
		const Eigen::Quaternion<T> q = RPYToQ(ori);
		const Eigen::Map<const Eigen::Matrix<T, 3, 1>> t(pos);
		const Eigen::Matrix<T, 3, 1> cam = q * p.cast<T>() + t;
		const Eigen::Matrix<T, 3, 1> proj = (P.cast<T>() * cam.homogeneous()).hnormalized();
		const T zero(0), one(1), two(2);
		for (uint i = 0; i < 2; i++)
			uv[i] = (proj[i] + one) / two;
		if (uv[0] < zero || uv[0] > one  //clip
			|| uv[1] < zero || uv[1] > one)
			//|| uv[2] < zero || uv[2] > one)
			return false;
		return true;
		//Logger::logProcess(__FUNCTION__);   //TODO remove logging
	}
}

