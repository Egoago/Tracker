#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace tr {
	class RasterPoint {
		glm::vec3 offsetPos;
		glm::vec2 uv;
		float angle;
		bool rendered;

	public:
		glm::vec3 pos;
		RasterPoint();
		RasterPoint(const glm::vec3 p, const glm::vec3 op);
		bool render(const glm::mat4& mvp);
		inline float getAngle() const { return angle; };
		inline glm::vec2 getUV() const { return uv; };

	};
}


