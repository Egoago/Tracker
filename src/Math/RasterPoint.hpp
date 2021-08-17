#pragma once
#include "../Misc/Base.hpp"

namespace tr {
	struct RasterPoint {
		vec3f offsetPos, pos;
		union {
			struct {
				vec2f uv;
				float angle;
			};
			const float indexData[3];
		};

		RasterPoint(const RasterPoint& other);
		RasterPoint(const vec3f p = vec3f::Zero(), const vec3f op = vec3f::Zero());
		bool render(const mat4f& mvp);
	};
}


