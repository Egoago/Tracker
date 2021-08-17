#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace tr {
	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> perspective(const Scalar fovy, const Scalar aspect, const Scalar zNear, const Scalar zFar) {
		Eigen::Transform<Scalar, 3, Eigen::Projective> tr;
		tr.matrix().setZero();
		Scalar radf = Scalar(EIGEN_PI) * fovy / Scalar(180);
		Scalar tan_half_fovy = std::tan(radf / Scalar(2));
		tr(0, 0) = Scalar(1) / (aspect * tan_half_fovy);
		tr(1, 1) = Scalar(1) / (tan_half_fovy);
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
	Eigen::Matrix<Scalar, 4, 4> rotate(const Scalar roll, const Scalar pitch, const Scalar yaw) {
		Eigen::Transform<Scalar, 3, Eigen::Affine> t;
		t.matrix().setIdentity();
		t.rotate(Eigen::AngleAxis<Scalar>(pitch, Eigen::Matrix<Scalar, 3, 1>::UnitX()));
		t.rotate(Eigen::AngleAxis<Scalar>(yaw, Eigen::Matrix<Scalar, 3, 1>::UnitY()));
		t.rotate(Eigen::AngleAxis<Scalar>(roll, Eigen::Matrix<Scalar, 3, 1>::UnitZ()));
		return t.matrix();
	}

	template<typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> rotate(const Eigen::Array<Scalar, 3, 1>& rpy) {
		return rotate(rpy[0], rpy[2], rpy[1]);
	}
}

