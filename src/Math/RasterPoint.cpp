#include "RasterPoint.hpp"
#include <Eigen/Geometry>
#include "../Misc/Log.hpp"

using namespace tr;

bool renderPoint(const vec3f p, const mat4f& mvp, vec2f& uv) {
	Eigen::Vector4f point = p.matrix().homogeneous();
	vec4f pPos = mvp * point;	//model -> NDC
	pPos /= pPos.w();								//Perspective divide
	pPos = (pPos + 1.0f) / 2.0f;				//NDC -> screen
	uv = pPos.head(2);
	if ((uv <  vec2f::Zero()).any() ||
		(uv >= vec2f::Ones()).any()) {
		Logger::warning("Rendering point out of bounds.");
		return false;
	}
	return true;
}

RasterPoint::RasterPoint(const RasterPoint& other) {
	Logger::warning("raster point copy");
	pos = other.pos;
	offsetPos = other.offsetPos;
	uv = other.uv;
	angle = other.angle;
}

#pragma warning(suppress : 26495)
RasterPoint::RasterPoint(const vec3f pos, const vec3f offsetPos) :
	pos(pos), offsetPos(offsetPos), uv(0), angle(0) {
}

bool tr::RasterPoint::render(const mat4f& mvp) {
	vec2f x, ox;
	if (!renderPoint(pos, mvp, x)) return false;
	if (!renderPoint(offsetPos, mvp, ox)) return false;
	const float _angle = orientation((x - ox).eval());
	if (isnan(_angle)) {
		//can be due to pixel out of bounds or nan angle.
		//it happens quite rarely but needs to be handled.
		Logger::warning("Unsuccessfull raster point render");
		return false;
	}
	uv = x;
	angle = _angle;
	return true;
}

vecm2f tr::RasterPoint::renderOffset(const mat4f& mvp) {
	vec2f x, ox;
	if (!renderPoint(pos, mvp, x)) return vecm2f(0);
	if (!renderPoint(offsetPos, mvp, ox)) return vecm2f(0);
	return (x - ox).eval().matrix().normalized();
}
