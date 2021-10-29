#pragma once
#include "../Misc/Base.hpp"

namespace tr {
	struct CameraParameters {
		double FOVx, FOVy, aspect;
		uvec2 resolution;
		static CameraParameters default() {
			CameraParameters cam;
			cam.aspect = 1.0;
			cam.FOVx = radian(90.0);
			cam.FOVy = radian(90.0);
			return cam;
		}
	};
}
