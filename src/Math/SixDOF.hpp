#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include "../Misc/Base.hpp"

namespace tr {
	struct SixDOF {
		union {
			struct {
				union {
					glm::vec<3, real> position;
					real posData[3];
				};
				union {
					glm::vec<3, real> orientation;
					real orData[3];
				};
			};
			real data[6];
		};

		SixDOF(glm::vec<3, real> position = glm::vec<3, real>(0.0f, 0.0f, 0.0f),
			glm::vec<3, real> orientation = glm::vec<3, real>(0.0f, 0.0f, 0.0f));

		glm::mat4 getModelTransformMatrix() const;

		friend std::ostream& operator<<(std::ostream& ost, const SixDOF& sDOF);
	};
}

