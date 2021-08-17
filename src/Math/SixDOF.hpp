#pragma once
#include <iostream>
#include "../Misc/Base.hpp"

namespace tr {
	struct SixDOF {
		union {
			struct {
				union {
					vec3f position;
					float posData[3];
				};
				union {
					vec3f orientation;
					float orData[3];
				};
			};
			float data[6];
		};

		SixDOF(vec3f position = vec3f(0.0f, 0.0f, 0.0f),
			vec3f orientation = vec3f(0.0f, 0.0f, 0.0f));

		SixDOF(const SixDOF& other) {
			position = other.position;
			orientation = other.orientation;
		}

		mat4f getModelTransformMatrix() const;

		friend std::ostream& operator<<(std::ostream& ost, const SixDOF& sDOF);
	};
}

