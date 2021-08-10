#pragma once
#include <glm/vec2.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace tr {
	inline float getOrientation(const glm::vec2 d) {
		float angle = glm::atan(d.y / d.x);

		if (angle < 0.0f)
			angle += glm::pi<float>();
		return angle;
	}
}
