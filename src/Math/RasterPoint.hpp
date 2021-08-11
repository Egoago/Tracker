#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace tr {
	struct RasterPoint {
		glm::vec3 offsetPos, pos;
		union {
			struct {
				glm::vec2 uv;
				float angle;
			};
			const float indexData[3];
		};

		RasterPoint();
		RasterPoint(const glm::vec3 p, const glm::vec3 op);
		bool render(const glm::mat4& mvp);
	};
}


