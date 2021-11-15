#pragma once
#include "../Misc/Base.hpp"

namespace tr {
	struct CameraParameters {
		double FOVy, aspect;
		uvec2 resolution;
		static CameraParameters default() {
			CameraParameters cam;
			cam.aspect = 1.0;
			cam.FOVy = radian(45.0);
			return cam;
		}
	};
}
