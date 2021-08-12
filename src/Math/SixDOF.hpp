#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>

namespace tr {
	struct SixDOF {
		union {
			struct {
				union {
					glm::vec3 position;
					float posData[3];
				};
				union {
					glm::vec3 orientation;
					float orData[3];
				};
			};
			float data[6];
		};

		SixDOF(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f));

		glm::mat4 getModelTransformMatrix() const;

		friend std::ostream& operator<<(std::ostream& ost, const SixDOF& sDOF);
	};
}

